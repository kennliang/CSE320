#include "protocol.h"
#include "debug.h"
#include <unistd.h>
#include "csapp.h"


/*
 * Send a packet, which consists of a fixed-size header followed by an
 * optional associated data payload.
 *
 * @param fd  The file descriptor on which packet is to be sent.
 * @param pkt  The fixed-size packet header, with multi-byte fields
 *   in host byte order
 * @param data  The data payload, or NULL, if there is none.
 * @return  zero in case of successful transmission, nonzero otherwise.
 *   In the latter case, errno is set to indicate the error.
 *
 * Multi-byte fields in the packet header are converted to network byte
 * order before sending.  The structure passed to this function may be
 * modified as a result of this conversion process.
 */
int proto_send_packet(int fd, MZW_PACKET *pkt, void *data)
{
   // debug("used1");
   // debug("\n\npkt type = %d pkt param1 =  %d pkt param2 = %d pkt param3 = %d pkt size = %d pkt timesec = %d pkt timensec = %d\n\n"
    //    , pkt -> type, pkt -> param1,pkt -> param2, pkt -> param3,pkt -> size, pkt -> timestamp_sec, pkt -> timestamp_nsec);

    int payload_size = pkt->size;

    pkt->size = htons(pkt ->size);
    pkt->timestamp_sec = htonl(pkt->timestamp_sec);
    pkt->timestamp_nsec = htonl(pkt->timestamp_nsec);

    int result;
    int size_packet = sizeof(MZW_PACKET);

    while ((result = write(fd,pkt,size_packet)))
    {
       // debug("result inside loop = %d",result);
        if( result == -1)
        {
            fprintf(stderr, "%s\n","write returned -1" );
            return 1;
        }
        size_packet = size_packet - result;
    }
    if(size_packet != 0)
    {
        fprintf(stderr, "%s\n", "incomplete in sending header");
        return 1;
    }

    if(payload_size != 0)
    {
        while ((result = write(fd,data,payload_size)))
        {
          //  debug("result inside loop = %d",result);
            if( result == -1)
            {
                fprintf(stderr, "%s\n","read returned -1" );
                return 1;
            }
            payload_size = payload_size - result;
        }
        if(payload_size != 0)
        {
            fprintf(stderr, "%s\n", "incomete in sending payload");
            return 1;
        }
    }


    return 0;
}

/*
 * Receive a packet, blocking until one is available.
 *
 * @param fd  The file descriptor from which the packet is to be received.
 * @param pkt  Pointer to caller-supplied storage for the fixed-size
 *   portion of the packet.
 * @param datap  Pointer to a variable into which to store a pointer to any
 *   payload received.
 * @return  zero in case of successful reception, nonzero otherwise.  In the
 *   latter case, errno is set to indicate the error.
 *
 * The returned structure has its multi-byte fields in host byte order.
 * If the returned payload pointer is non-NULL, then the caller has the
 * responsibility of freeing that storage.
 */
int proto_recv_packet(int fd, MZW_PACKET *pkt, void **datap)
{
   // debug("used");
   // debug("pkt type = %d pkt param1 =  %d pkt param2 = %d pkt param3 = %d pkt size = %d pkt timesec = %d pkt timensec = %d"
     //   , pkt -> type, pkt -> param1,pkt -> param2, pkt -> param3,pkt -> size, pkt -> timestamp_sec, pkt -> timestamp_nsec);

    int size_packet = sizeof(MZW_PACKET);

    int result;
    debug("before read loop");

    while ((result = read(fd,pkt,size_packet)) && size_packet != 0)
    {
        debug("result inside loop = %d",result);
        if( result == -1)
        {
            fprintf(stderr, "%s\n","read returned -1" );
            return 1;
        }
        size_packet = size_packet - result;
    }
    debug("result outside loop = %d",result);
    if(size_packet != 0)
    {
        fprintf(stderr, "%s\n", "incomplete receiving header");
        return 1;
    }


   // debug("size_packet = %d result = %d",size_packet,result);
   //  debug("pkt type = %d pkt param1 =  %d pkt param2 = %d pkt param3 = %d pkt size = %d pkt timesec = %d pkt timensec = %d"
     //   , pkt -> type, pkt -> param1,pkt -> param2, pkt -> param3,pkt -> size, pkt -> timestamp_sec, pkt -> timestamp_nsec);

     pkt->size = ntohs(pkt ->size);
     pkt->timestamp_sec = ntohl(pkt->timestamp_sec);
     pkt->timestamp_nsec = ntohl(pkt->timestamp_nsec);
     // debug("pkt type = %d pkt param1 =  %d pkt param2 = %d pkt param3 = %d pkt size = %d pkt timesec = %d pkt timensec = %d"
      //  , pkt -> type, pkt -> param1,pkt -> param2, pkt -> param3,pkt -> size, pkt -> timestamp_sec, pkt -> timestamp_nsec);

    //there is payload
    if(pkt->size != 0)
    {
        debug("there is payload");
        *datap = Calloc(1,pkt->size);

        int payload_size = pkt->size;
        while ((result = read(fd,*datap,payload_size)))
        {
            //debug("result inside loop = %d",result);
            if( result == -1)
            {
                fprintf(stderr, "%s\n","read returned -1" );
                return 1;
            }
            payload_size = payload_size - result;
        }
        if(payload_size != 0)
        {
            fprintf(stderr, "%s\n", "incomplete recieving payload");
            return 1;
        }
    }

    return 0;
}