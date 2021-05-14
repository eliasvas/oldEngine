#ifndef NET_H
#define NET_H
#include "tools.h"
#define PLATFORM_WINDOWS  1
#define PLATFORM_MAC      2
#define PLATFORM_UNIX     3

#if defined(_WIN32)
#define PLATFORM PLATFORM_WINDOWS
#pragma comment( lib, "wsock32.lib" )
#elif defined(__APPLE__)
#define PLATFORM PLATFORM_MAC
#else
#define PLATFORM PLATFORM_UNIX
#endif
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;

#if PLATFORM == PLATFORM_WINDOWS
#include <winsock2.h>
#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#endif
internal u32 initialize_sockets(void)
{
#if PLATFORM == PLATFORM_WINDOWS
    WSADATA wsadata;
    return WSAStartup(MAKEWORD(2,2), &wsadata) == NO_ERROR;
#else 
    return TRUE;
#endif
}
internal void shutdown_sockets(void)
{
#if PLATFORM == PLATFORM_WINDOWS
    WSACleanup();
#endif
}

typedef struct PacketHeader
{
    u32 pid;
    u32 sequence;
    u32 ack;
    u32 ack_bits;//if bit n active, sequence[ack - n] is acked
}PacketHeader;
typedef struct Address
{
    u32 address;
    u16 port;
}Address;

internal Address address_create(u32 address, u32 port)
{
    Address res;
    res.address = address;
    res.port = port;
    return res;
}
internal u32 abcd_to_addr(u8 a, u8 b, u8 c, u8 d)
{
    u32 address = (a << 24)|(b << 16)|(c << 8)|d;
    
    return address;
}

typedef struct Socket
{
    i32 handle;
}Socket;

internal u32 socket_set_non_blocking(Socket *s)
{
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
    i32 non_blocking = 1;
    if (fcntl( s->handle, F_SETFL, O_NONBLOCK, non_blocking ) == -1)
    {
        printf( "failed to set non-blocking\n" );
        return FALSE;
    }
#elif PLATFORM == PLATFORM_WINDOWS
    DWORD non_blocking = 1;
    if (ioctlsocket(s->handle, FIONBIO,&non_blocking) != 0)
    {
        printf( "failed to set non-blocking\n" );
        return FALSE;
    }
#endif
    return TRUE;
}

internal u32 socket_open(Socket *s, u16 port)
{
    i32 handle = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if (handle <=0)
    {
        printf("failed to create socket!\n");
        return FALSE;
    }
    s->handle = handle;
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons((u16)port); //host to network

    if (bind(handle, (sockaddr*) &address, sizeof(sockaddr_in)) < 0)
    {
        printf("handle: %i\n", handle);
        printf("failed to bind socket\n");
        return FALSE;
    }
    if (!socket_set_non_blocking(s))return FALSE;
    return TRUE;
}

internal void socket_close(Socket *s)
{
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
    close( socket );
#elif PLATFORM == PLATFORM_WINDOWS
    closesocket( socket );
#endif
}

internal b32 socket_is_open(Socket *s)
{
    return TRUE;
}

internal u32 socket_send(Socket * s, Address destination, void *packet_data, i32 packet_size)
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(destination.address);
    addr.sin_port = htons(destination.port);

    i32 sent_bytes = sendto(s->handle, (char*)packet_data, packet_size, 0, (sockaddr*)&addr, sizeof(sockaddr_in));
    if (sent_bytes != packet_size)
    {
        printf( "failed to send packet(local)\n" );
        printf( "packet size: %i, sent bytes: %i\n", packet_size, sent_bytes);
        return FALSE;
    }
    return TRUE;
}

internal u32
socket_recieve(Socket *s, Address *sender, void *data, i32 size)
{
 while (TRUE)
 {
    sockaddr_in from;
    u32 from_len = sizeof(from);

    i32 bytes = recvfrom(s->handle, (char*)data, size, 0, (sockaddr*)&from, &from_len);

    if ( bytes <= 0 ) break;

    u32 from_address = ntohl( from.sin_addr.s_addr );

    u32 from_port = ntohs( from.sin_port );
    *sender = address_create(from_address, (u16)from_port);
    // process received packet
    return TRUE;
 }
 return FALSE;
}


#define PACKET_SIZE 67 * sizeof(u8) 

typedef struct Message
{
    u32 message_id;
    u32 number_of_packets;
    b32 active;
    u32 *acks; //array of acks for eack packet in message
    void *data;//optional
}Message;

internal Message message_init(u32 n, void *data)
{
    Message res;
    res.number_of_packets = n;
    res.acks = ALLOC(sizeof(u32) * n);
    res.data = data;
    res.message_id = random();
    return res;
}

typedef struct NetworkManager
{
    u32 sender_acks[PACKET_SIZE * 1000];
    u32 sender_timers[PACKET_SIZE * 1000];
    u32 sender_subdivisions;
    u32 sender_id; //every sender sends one message at a time so there is no need for id per message
    b32 sender_busy;
    Address addr_to_send;
    void *data_to_send;
    //these are essential
    u8 *recv_buf;
    Socket s;
    u32 active_id;
}NetworkManager;
internal NetworkManager net;

internal void network_manager_initialize(void)
{
    initialize_sockets();
    socket_open(&net.s, 30001);
    net.recv_buf = ALLOC(sizeof(u8) * PACKET_SIZE * 1000);
    net.sender_id = random();
    //set pids = INT_MAX;
}
internal void network_manager_shutdown(void)
{
    shutdown_sockets();
}

u8 local_memory[PACKET_SIZE * 3000];

internal u32 network_manager_send_nosecure(Address destination, void *packet_data, i32 packet_size)
{ 
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(destination.address);
    addr.sin_port = htons(destination.port);



    u32 number_of_packets_to_be_sent = packet_size / (PACKET_SIZE - sizeof(PacketHeader));
    //printf("num of (cut) packets to be sent: %i\n", number_of_packets_to_be_sent);
    net.data_to_send = packet_data;
    u32 id = random();
    for (u32 i = 0; i < number_of_packets_to_be_sent; ++i)
    {
        PacketHeader hdr = {net.sender_id, i,0,0};
        memcpy((void*)local_memory, (void*)(&hdr),sizeof(PacketHeader));
        memcpy((void*)((u64)local_memory + (u64)(sizeof(PacketHeader))), (void*)((u64)net.data_to_send+ (u64)(i * (PACKET_SIZE - sizeof(PacketHeader)))),PACKET_SIZE);

        socket_send(&net.s, destination, local_memory, PACKET_SIZE);
    }
    return TRUE;
}


internal void network_manager_recieve_nosecure(void)
{
    i32 i = 0;
    f32 accum = 0.f;
    f32 accum_lim = 1.f;
    while (TRUE)
    {
        accum += 0.0001;
        Address sender;
        i32 bytes_read = socket_recieve(&net.s, &sender, (void*)((u64)local_memory + (u64)(i * PACKET_SIZE)), PACKET_SIZE);
        if (bytes_read)
        {
            accum = 0.f;
            ++i;
        }
        if (accum > accum_lim)break; //if no packets where recieved for a long long time
    }

    PacketHeader hdr;
    for (u32 j = 0; j < i; ++j)
    {
        memcpy((void*)(&hdr), (void*)((u64)local_memory + (u64)(j * PACKET_SIZE)), sizeof(PacketHeader));
        if (net.active_id ==0)net.active_id = hdr.pid;
        else if (net.active_id != hdr.pid)continue;
        memcpy((void*)((u64)net.recv_buf + (u64)(hdr.sequence* (PACKET_SIZE - sizeof(PacketHeader)))), (void*)((u64)local_memory + (u64)(j * PACKET_SIZE) + sizeof(PacketHeader)), PACKET_SIZE - sizeof(PacketHeader));
    }
}
internal u32 network_manager_send(Address destination, void *packet_data, i32 packet_size)
{ 
    if (net.sender_busy)return FALSE;
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(destination.address);
    addr.sin_port = htons(destination.port);



    u32 number_of_packets_to_be_sent = 1 +  packet_size / (PACKET_SIZE - sizeof(PacketHeader));
    //printf("num of (cut) packets to be sent: %i\n", number_of_packets_to_be_sent);
    net.sender_subdivisions = number_of_packets_to_be_sent;
    net.data_to_send = packet_data;
    net.addr_to_send = destination;
    net.sender_busy = TRUE;
    u32 id = random();
    for (u32 i = 0; i < number_of_packets_to_be_sent; ++i)
    {
        PacketHeader hdr = {net.sender_id, i,INT_MAX,0};
        net.sender_acks[i] = 0; //set all acknoledges to false
        net.sender_timers[i] = time(NULL); //set all the timers to the time they were sent (for timeout)
        memcpy((void*)local_memory, (void*)(&hdr),sizeof(PacketHeader));
        memcpy((void*)((u64)local_memory + (u64)(sizeof(PacketHeader))), (void*)((u64)net.data_to_send+ (u64)(i * (PACKET_SIZE - sizeof(PacketHeader)))),PACKET_SIZE);

        //if (i != 2)
        socket_send(&net.s, destination, local_memory, PACKET_SIZE);
    }
    return TRUE;
}
internal u32 network_manager_send_ACK(Address destination, PacketHeader *hdr_data)
{ 
    return socket_send(&net.s, destination, hdr_data, PACKET_SIZE);
}

internal u32 network_manager_send_disconnect(Address destination)
{ 
    PacketHeader h = (PacketHeader){666, 666, 666, 666};
    return socket_send(&net.s, destination, &h, PACKET_SIZE);
}
internal void network_manager_recieve(void)
{
    i32 i = 0;
    f32 accum = 0.f;
    f32 accum_lim = 1.f;
    Address senders[100];
    //[1] We recieve all the messages waiting in the queue (maybe tweak accum_lim in case we get ALOT of responses? YES)
    while (TRUE)
    {
        accum += 0.0001;
        i32 bytes_read = socket_recieve(&net.s, &senders[i], (void*)((u64)local_memory + (u64)(i * PACKET_SIZE)), PACKET_SIZE);
        if (bytes_read)
        {
            accum = 0.f;
            ++i;
        }
        if (accum > accum_lim)break; //if no packets where recieved for a long long time
    }

    PacketHeader hdr;
    //[2] here we handle all the messages recieved above, we update sender ACK tables and send out ACKS if server recieved segment alright
    for (u32 j = 0; j < i; ++j)
    {
        memcpy((void*)(&hdr), (void*)((u64)local_memory + (u64)(j * PACKET_SIZE)), sizeof(PacketHeader));
        if (hdr.ack == 666 && hdr.pid == 666 && hdr.ack_bits == 666)//if all header is 666 this is a disconnection signal from the sender side!
        {
            printf("SERVER DISCONNECTED from sender\n"); 
            net.active_id = 0;
            continue;
        }
        else if (hdr.ack != INT_MAX)
        {//ACK resolution
            u32 seq_acked = hdr.ack;
            net.sender_acks[seq_acked] = 1;
        }
        else
        {//typical message
            printf("SERVER RECV pid: %i, ack: %i, seq: %i\n", hdr.pid, hdr.ack, hdr.sequence);
            if (net.active_id ==0)net.active_id = hdr.pid;
            else if (net.active_id != hdr.pid)continue;
            memcpy((void*)((u64)net.recv_buf + (u64)(hdr.sequence* (PACKET_SIZE - sizeof(PacketHeader)))), (void*)((u64)local_memory + (u64)(j * PACKET_SIZE) + sizeof(PacketHeader)), PACKET_SIZE - sizeof(PacketHeader));

            //send an ACK with the sequence number of the segment recieved
            hdr.ack = hdr.sequence;
            network_manager_send_ACK(senders[j],&hdr);
        }
    }
    //[3] check to see if any sender side segment response is timed out and resend lost segment
    for (u32 j = 0; j < net.sender_subdivisions && net.sender_busy; ++j)
    {
            if (!net.sender_acks[j] && ((u32)time(NULL) > (u32)net.sender_timers[j]))
            {
                   //TODO: replace all this bullshit by a socket send
                   PacketHeader hdr = {net.active_id, j,INT_MAX,0};
                   net.sender_timers[i] = time(NULL); //set all the timers to the time they were sent (for timeout)
                   memcpy((void*)local_memory, (void*)(&hdr),sizeof(PacketHeader));
                   memcpy((void*)((u64)local_memory + (u64)(sizeof(PacketHeader))), (void*)((u64)net.data_to_send + (u64)(j * (PACKET_SIZE - sizeof(PacketHeader)))),PACKET_SIZE);
                   socket_send(&net.s, net.addr_to_send, local_memory, PACKET_SIZE);
                   printf("resend [%i]\n", j);
            }
            if (!net.sender_acks[j])break;
            if (j == net.sender_subdivisions-1)net.sender_busy = FALSE;
            if (j == net.sender_subdivisions-1)network_manager_send_disconnect(net.addr_to_send); //but what happens if disconnect comes before a dupe? ihave officially become paranoid
    }
}


#endif

