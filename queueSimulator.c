#include "queueSimulator.h"

//constants
uint TICKS = 100000;
uint lambda = 150;
uint pack_length = 2000;
uint service_rate = 1000000;
uint buffer_inf = 1;
uint buffer_limit = 0;

//data for each simulation 
int tick_dur = 1;
int msPerTick = 1;
uint totPacketsGenerated = 0;
uint totPacketsDropped = 0;
uint totPacketsServed = 0;
uint ticksIdle = 0;
double avgSojournTicks = 0.0;
double avgPackets = 0.0;
double p_idle = 0.0;
double p_loss = 0.0;

//average data from all simulations
double avg_avgSojournTicks = 0.0;
double avg_avgPackets = 0.0;
double avg_p_idle = 0.0;
double avg_p_loss = 0.0;

uint getNextTickGen() {
    double u = 0.0;
    uint nextTickGen = 0;
    while (nextTickGen == 0) {
        u = (double)rand() / (double)RAND_MAX;
        nextTickGen = (uint)((-1.0/((double)lambda))*log(1.0-u) * (double)(1000/msPerTick));
    }
    return nextTickGen;
}

void generate(uint tick) {
    if (buffer_inf == 1 || queue.size < buffer_limit) {
        Packet * packet = (Packet *) malloc(sizeof(Packet));
        if (packet == NULL) {
            printf("ERROR: could not allocate a packet");
            exit(1);
        }
        packet->tick_generated = tick;
        packet->bits_served = 0;
        packet->below = NULL;

        totPacketsGenerated++;
        Queue_add(packet); 
    }
    else {
        totPacketsDropped++;
    }
}

void serve(uint tick) {
    if (queue.size != 0) {
        Packet * head = queue.head;
        uint bitsPerTick = (service_rate / 1000) * msPerTick;
        if (pack_length - (head->bits_served) > bitsPerTick) {
            head->bits_served = (head->bits_served) + bitsPerTick;
        }
        else {
            uint sojournTicks = (tick - (head->tick_generated)) + 1;
            avgSojournTicks = ((avgSojournTicks * (double)totPacketsServed) + (double)sojournTicks) / ((double)(totPacketsServed + 1));
            free(Queue_remove());
            totPacketsServed++;
        }
    }
    else {
        ticksIdle++;
    }
}

void simulate() {
	
    //reset on each simulation
    totPacketsGenerated = 0;
    totPacketsDropped = 0;
    totPacketsServed = 0;
    ticksIdle = 0;
    avgSojournTicks = 0.0;
    avgPackets = 0.0;
    p_idle = 0.0;
    p_loss = 0.0;

    Queue_init();
    uint tickGen = 1;
	
    for (uint tick = 0; tick < TICKS; tick++) {
        if (tick == tickGen) {
            generate(tick);
            tickGen = tick + getNextTickGen();
        }
        serve(tick);
		
        avgPackets = ((avgPackets * (double)tick) + (double)queue.size) / ((double)(tick+1));
    }
	
    p_loss = (double)totPacketsDropped / (double)totPacketsGenerated;
    p_idle = (double)ticksIdle / (double)TICKS;

    Queue_clear();
}

int main( int argc, char *argv[] ) {
    long long num;
    char * aEnd;

    if (argc < 5){
        printf("Too few arguments. Expecting [TICKS] [lambda] [L] [C] or [TICKS] [lambda] [L] [C] [K]");
        return 0;
    }
    if (argc > 6){
        printf("Too many arguments. Expecting [TICKS] [lambda] [L] [C] or [TICKS] [lambda] [L] [C] [K]\n");
        return 0;
    }

    for (int i = 1; i < argc; i++){
        num = strtoll(argv[i], &aEnd, 10);
        if (aEnd == argv[i] || *aEnd != '\0' || num<0 || num>4294967295){
            printf("Argument number %d, %s is not a valid unsigned integer value", i, argv[i]);
            return 0;
        }
        switch (i){
            case 1:
                TICKS = (uint)num;
                break;
            case 2:
                lambda = (uint)num;
                break;
            case 3:
                pack_length = (uint)num;
                break;
            case 4:
                service_rate = (uint)num;
                break;
            case 5:
                buffer_limit = (uint)num;
                buffer_inf = 0;
                break;
        }
    }

    int numSimRuns = 5;     //need to use 5-10
    for (int i = 0; i < numSimRuns; i++) {
        simulate();
        avg_avgPackets = (((double)avg_avgPackets * (double)i) + (double)avgPackets) / ((double)(i+1));
        avg_avgSojournTicks = (((double)avg_avgSojournTicks * (double)i) + (double)avgSojournTicks) / ((double)(i+1));
        avg_p_idle = (((double)avg_p_idle * (double)i) + (double)p_idle) / ((double)(i+1));
        avg_p_loss = (((double)avg_p_loss * (double)i) + (double)p_loss) / ((double)(i+1));
    }

    printf("E[N]:   %.6f\n", avg_avgPackets);
    printf("E[T]:   %.6fs\n", (avg_avgSojournTicks * (double)1000));
    printf("p_idle: %.6f\n", avg_p_idle); 
    if (buffer_inf == 0){
        printf("p_loss: %.6f\n", avg_p_loss);
    }
    
    return 0;
}
