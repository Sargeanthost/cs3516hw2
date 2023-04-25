#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project2.h"

#define WAITING 0
#define SENDING 1

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

int current_state;

struct msg_queue {
    struct msg message;
    struct msg_queue *next;
};

struct msg_queue *queue_head;
struct msg_queue *queue_tail;

struct pkt *current_packet;

int run_seq = 0;

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

void addToQ(struct msg message) {
    struct msg_queue *queue = malloc(sizeof(struct msg_queue));
    memcpy(&queue->message, &message, sizeof(struct msg));
    queue->next = NULL;

    if(queue_head == NULL){
        //first item
        queue_head = queue;
        queue_tail = queue;
    } else {
        //next is done first so that the current last node is linked to this, new last node
        queue_tail->next = queue;
        //no longer the last node, the new one is
        queue_tail = queue;
    }
}

struct msg *popQueue(){
    if(queue_head == NULL){
        return NULL;
    }

    struct msg *message = malloc(sizeof(struct msg));
    memcpy(message, &queue_head->message, sizeof(struct msg));

    //move over queue
    struct msg_queue *prev_head = queue_head;
    queue_head = queue_head->next;//the next one
    free(prev_head);

    return message;
}

void sendBSide(struct msg *message) {
    //global packet so that every function knows what is currently being processed.
    current_packet = malloc(sizeof(struct pkt));
    int checksum = compute_checksum(message->data, 0, run_seq);
    current_packet->acknum = 0;
    current_packet->seqnum = run_seq;
    current_packet->checksum = checksum;
    strcpy(current_packet->payload, message->data);

    tolayer3(AEntity, *current_packet);
    startTimer(AEntity, 100);

    current_state = WAITING;
}

/* 
 * A_output(message), where message is a structure of type msg, containing 
 * data to be sent to the B-side. This routine will be called whenever the 
 * upper layer at the sending side (A) has a message to send. It is the job 
 * of your protocol to ensure that the data in such a message is delivered
 * in-order, and correctly, to the receiving side upper layer.
 */
void A_output(struct msg message) {
    if (current_state == WAITING) {
        addToQ(message);
    } else {
        sendBSide(&message);
    }
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
    if(current_state == SENDING){
        puts("still sending packet");
        return;
    }
    stopTimer(AEntity);
//handle the ack
//if corrupt or not 1 in acknum field, resend
//1 means bad
    if (packet.acknum == 1 || compute_checksum(packet.payload, packet.acknum, packet.seqnum) != packet.checksum) {
        tolayer3(AEntity, *current_packet);
        startTimer(AEntity, 100);
    } else if (packet.acknum == 0){

        //we know that it sent correctly, so send the next packet.
        free(current_packet);
        current_state = SENDING;

        run_seq = !run_seq; //since we are not using go back n, we can just swap what the sequence is
        if(queue_head != NULL){
            struct msg *message = popQueue();
            sendBSide(message);
            free(message);//have to free the memory so left as 3 instead of 1 line
        }
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
    //just to layer 3 because we are simply resending the same packet, not the next packet, so no need for sendBSide
    tolayer3(AEntity, *current_packet);
    startTimer(AEntity, 100);
}

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
    queue_head = NULL;
    queue_tail = NULL;
    current_state = SENDING;
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
    char *empty_buffer = malloc(sizeof(char) * MESSAGE_LENGTH);
    memset(empty_buffer, 0, MESSAGE_LENGTH);

    send_packet.acknum = compute_checksum(packet.payload,packet.acknum,packet.seqnum) != packet.checksum;
    send_packet.seqnum = run_seq;
    memcpy(send_packet.payload, empty_buffer, MESSAGE_LENGTH);
    send_packet.checksum = compute_checksum(empty_buffer, compute_checksum(packet.payload,packet.acknum,packet.seqnum) != packet.checksum, run_seq);

    tolayer3(BEntity, send_packet);

    //check to layer 5. the above runs no matter what, but layer 5 should only receive correct data
    if(packet.seqnum != run_seq || compute_checksum(packet.payload, packet.acknum, packet.seqnum) != packet.checksum){
        return;
    }

    //if we are good...

    struct msg *message = malloc(sizeof(struct msg));
    strcpy(message->data, packet.payload);
    tolayer5(BEntity, *message);
    run_seq = !run_seq;
    free(message);
}

/*
 * B_timerinterrupt()  This routine will be called when B's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void B_timerinterrupt() {

}

/* 
 * The following routine will be called once (only) before any other   
 * entity B routines are called. You can use it to do any initialization 
 */
void B_init() {
    //nothing
}

