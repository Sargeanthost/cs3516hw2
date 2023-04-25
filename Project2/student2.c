#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project2.h"

/* ***************************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for unidirectional or bidirectional
   data transfer protocols from A to B and B to A.
   Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets may be delivered out of order.

   Compile as gcc -g project2.c student2.c -o p2
**********************************************************************/

int base = 0; //this is the last known acked seq number
int run_seqnum = 0; //this is the last sent packet's seq.
int cur_ack = 0; //just used for acking packets

typedef struct packet_node{
    struct pkt *packet;
    struct packet_node *next_node;
} packet_node;

typedef struct {
    packet_node *head;
    packet_node *tail;
} queue;

queue waiting_queue;

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/* 
 * The routines you will write are detailed below. As noted above, 
 * such procedures in real-life would be part of the operating system, 
 * and would be called by other procedures in the operating system.  
 * All these routines are in layer 4.
 */
int compute_checksum(char *data, int acknum, int seqnum) {
    int i, checksum = 0;

    for (i = 0; i < MESSAGE_LENGTH; i++) {
        checksum += (int) (data[i]) * i;
    }

    checksum += acknum * 21;
    checksum += seqnum * 22;

    return checksum;
}

void addToQ(struct pkt *packet) {
    packet_node *new_packet = NULL;
    new_packet->packet = packet;
    new_packet->next_node = NULL;

    if (waiting_queue.head == NULL) {
        waiting_queue.head = new_packet;
        waiting_queue.tail = new_packet;
    } else {
        waiting_queue.tail->next_node = new_packet;
    }
}

struct pkt getNextPacket(){
    for(packet_node *node = waiting_queue.head; node != NULL; node = node->next_node){
        if (node->packet->seqnum == base){
            return *(node->packet);
        }
    }
    //TODO this might be a problem
    return *waiting_queue.head->packet;
}

/* 
 * A_output(message), where message is a structure of type msg, containing 
 * data to be sent to the B-side. This routine will be called whenever the 
 * upper layer at the sending side (A) has a message to send. It is the job 
 * of your protocol to ensure that the data in such a message is delivered
 * in-order, and correctly, to the receiving side upper layer.
 */
void A_output(struct msg message) {
    //seq num is the 0,n number where n is bytes and acknum is 0 or 1
    struct pkt *send_packet = malloc(sizeof(struct pkt));
    send_packet->acknum = 0;
    send_packet->seqnum = run_seqnum;
    send_packet->checksum = compute_checksum(message.data, cur_ack, run_seqnum);
    strcpy(send_packet->payload, message.data);
    addToQ(send_packet);

    tolayer3(AEntity, getNextPacket());

    run_seqnum += sizeof(struct pkt);
    //TODO right timeout?
    startTimer(AEntity, 100);
}

/*
 * Just like A_output, but residing on the B side.  USED only when the 
 * implementation is bi-directional.
 */
void B_output(struct msg message) {
//nothing here
}

/* 
 * A_input(packet), where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the B-side (i.e., as a result 
 * of a tolayer3() being done by a B-side procedure) arrives at the A-side. 
 * packet is the (possibly corrupted) packet sent from the B-side.
 */
void A_input(struct pkt packet) {
//handle the ack
//if corrupt or not 1 in acknum field, do nothing and wait for timeout
    if (packet.acknum == 1 && compute_checksum(packet.payload, packet.acknum, packet.seqnum) == packet.checksum) {
        base = packet.seqnum;//since the seq num of the ack is the
        stopTimer(AEntity);
    }
//else if not corrupt and ack is 1, stop timer
}

/*
 * A_timerinterrupt()  This routine will be called when A's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt() {
//    stopTimer(AEntity);
//    struct pkt send_pkt =
//            tolayer3(AEntity, send_pkt);
//    //resend
//    startTimer(AEntity,)
}

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
    waiting_queue.head = NULL;
    waiting_queue.tail = NULL;
}


/* 
 * Note that with simplex transfer from A-to-B, there is no routine  B_output() 
 */

/*
 * B_input(packet),where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the A-side (i.e., as a result 
 * of a tolayer3() being done by a A-side procedure) arrives at the B-side. 
 * packet is the (possibly corrupted) packet sent from the A-side.
 */
void B_input(struct pkt packet) {
    //if packet is corrupt, do nothing and wait for timeout and handle in b_timerinterrupt,
    // else send ack packet with 1
    struct pkt send_packet;
    send_packet.acknum = 1;
    send_packet.seqnum = packet.seqnum;
    strcpy(send_packet.payload, packet.payload);
    send_packet.checksum = compute_checksum(packet.payload, 1, packet.seqnum);
    tolayer3(BEntity, send_packet);
    startTimer(BEntity, 100);
}

/*
 * B_timerinterrupt()  This routine will be called when B's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void B_timerinterrupt() {
    puts("hi");
}

/* 
 * The following routine will be called once (only) before any other   
 * entity B routines are called. You can use it to do any initialization 
 */
void B_init() {
    puts("ectra hi");
}

