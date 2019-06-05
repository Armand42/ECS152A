// WLAN Simulation
// Yara Fawaz   912243010
// Armand Nasseri   912679383
// Jennifer Nguyen  914995612

#include <iostream>
#include <iomanip>
#include <queue>
#include <math.h>

#define MAXBUFFER 30
#define MAXPACKETSIZE 1544
#define ACKPACKETSIZE 64
#define NUM_HOSTS 10
#define T 20
#define SUCCESS true
#define FAILURE false
#define SIFS .00005
#define DIFS .0001
#define BACKOFFTIME 0.1

using namespace std;
enum eventType
{
    arrival,
    departure,
    backoff
};

// Function provided by assignment
double negativeExponentiallyDistributedTime(double rate)
{
    double u;
    u = drand48();
    return ((-1 / rate) * log(1 - u));
}

// Event Node
struct Event
{
  public:
    double eventTime;
    //arrival event: host it's arriving at
    //departure event: host it's going to
    //backoff event: host is -1
    bool isAck;
    int host;
    int bytes;
    eventType type;
    Event *next;
    Event *prev;
};

Event **arrivalEvent(Event **event, double arrivalRate, double currentTime, int srcHost)
{
    (*event)->eventTime = negativeExponentiallyDistributedTime(arrivalRate) + currentTime;
    (*event)->type = arrival;
    (*event)->host = srcHost;
    (*event)->next = (*event)->prev = NULL;
    (*event)->bytes = 0;
    return event;
}

Event **backoffEvent(Event **event, double currentTime)
{
    (*event)->eventTime = BACKOFFTIME + currentTime;
    (*event)->type = backoff;
    (*event)->host = -1;
    (*event)->next = (*event)->prev = NULL;
    (*event)->bytes = 0;
    return event;
}

Event **departureEvent(Event **event, double serviceTime, double currentTime, int srcHost, bool isAck, int bytes)
{
    if (isAck)
        (*event)->eventTime = serviceTime + currentTime + SIFS;
    else
        (*event)->eventTime = serviceTime + currentTime + DIFS;

    (*event)->type = departure;
    int destHost;
    do
    {
        destHost = rand() % NUM_HOSTS;
    } while (destHost != srcHost);
    (*event)->host = destHost;
    (*event)->next = (*event)->prev = NULL;
    (*event)->isAck = isAck;
    (*event)->bytes = bytes;
    return event;
}

// Global Event list: Implemented as a double nexted list
class GEL
{
  public:
    Event *head;
    Event *tail;
    GEL();
    bool insertEvent(Event **newEvent);
    Event *removeFirstEvent();
    void printGEL();
    bool isTransmittingPacket();
};

GEL::GEL()
{
    head = tail = NULL;
}

// Insert an event at the proper location of the nexted list
bool GEL::insertEvent(Event **newEvent)
{
    if (head == NULL && tail == NULL)
    {
        head = tail = *newEvent;
        return SUCCESS;
    }
    Event *index = head;
    while (index != NULL && index->eventTime < (*newEvent)->eventTime)
    {
        index = index->next;
    }
    if (index == head)
    {
        (*newEvent)->next = head;
        head->prev = *newEvent;
        head = *newEvent;
    }
    else if (index == NULL)
    {
        (*newEvent)->prev = tail;
        tail->next = *newEvent;
        tail = *newEvent;
    }
    else
    {
        (*newEvent)->next = index;
        (*newEvent)->prev = index->prev;
        index->prev->next = *newEvent;
        index->prev = *newEvent;
    }

    return SUCCESS;
}
// Extracts the first element of the global event list
Event *GEL::removeFirstEvent()
{
    Event *first = head;
    if (head == tail)
    {
        head = tail = NULL;
        // printGEL();
        return first;
    }

    head = first->next;
    head->prev = NULL;

    return first;
}
// Helper function to print all events in our list
void GEL::printGEL()
{
    Event *current = head;
    cout << "\nGEL: ";
    while (current != NULL)
    {
        printf("%s EventTime: %f -> ", current->type == arrival ? "Arrival" : "Departure", current->eventTime);

        current = current->next;
    }
}

struct Packet
{
  public:
    int frameSize;
    bool isAck;
    int host; //where i am sending this packet to
    Packet *next;
};

void acknowledgementPacket(Packet **packet, int host)
{
    (*packet)->frameSize = ACKPACKETSIZE * 8; // bytes->bits
    (*packet)->host = host;
    (*packet)->next = NULL;
    (*packet)->isAck = true;
}

void packet(Packet **packet, int frameSize, int host)
{
    (*packet)->frameSize = frameSize * 8; // bytes->bits
    (*packet)->host = host;
    (*packet)->next = NULL;
    (*packet)->isAck = false;
}

class Host
{
  public:
    Packet *head;
    Packet *tail;
    int backoff;
    Host();
    Packet *pop();
    void push(Packet *p);
    void pushFront(Packet **p);
};

Host::Host()
{
    head = tail = NULL;
    backoff = -1;
}

Packet *Host::pop()
{
    if (head == NULL)
        return NULL;
    Packet *temp = head;
    head = head->next;
    if (tail == temp)
        tail = NULL;
    return temp;
}

void Host::push(Packet *p)
{
    if (head == NULL && tail == NULL)
    {
        head = tail = p;
    }
    else
    {
        tail->next = p;
        tail = p;
    }
}

void Host::pushFront(Packet **p)
{
    (*p)->next = head;
    head = *p;
    if (tail == NULL)
    {
        tail = head;
    }
}

int main()
{
    const double channelRate = 10000000; //Mbps

    queue<Packet *> queue;
    cout << "***STATISTICS***\n";
    cout << endl;
    double arrivalRates[7] = {0.1, 0.2, 0.4, 0.5, 0.6, 0.8, 0.9};
    for (int i = 0; i < 7; i++)
    {
        int totalBytesTransmitted = 0;
        bool channelBusy = false;
        double delay = 0;
        //Time and counters for statistics.
        double currentTime = 0, serverBusyTime = 0, totalNumberOfPackets = 0, droppedPackets = 0, intervalTime = 0;
        //Rates
        double serviceRate = 1;               //mu
        double arrivalRate = arrivalRates[i]; //lamda

        //Data Structure Init
        GEL gel = GEL();
        Host hosts[NUM_HOSTS];

        for (int i = 0; i < NUM_HOSTS; i++)
        {
            //CREATE EVENTS FOR THIS
            Event *event = new Event;
            arrivalEvent(&event, arrivalRate, currentTime, i);
            gel.insertEvent(&event);
        }

        Event *event = new Event;
        backoffEvent(&event, currentTime);
        gel.insertEvent(&event);

        for (int i = 0; i < 100000; i++)
        {
            Event *currentEvent = gel.removeFirstEvent();
            intervalTime = currentEvent->eventTime - currentTime;
            currentTime = currentEvent->eventTime;
            if (currentEvent->type == arrival)
            {
                // Generating the next arrival
                double arrivalTime = negativeExponentiallyDistributedTime(arrivalRate);
                int frameSize = (int)(negativeExponentiallyDistributedTime(serviceRate) * MAXPACKETSIZE) % MAXPACKETSIZE;

                Event *newArrival = new Event;
                arrivalEvent(&newArrival, arrivalRate, currentTime, currentEvent->host);
                gel.insertEvent(&newArrival);
                // Processing the Arrival Event

                //adding the newly received packet to the queue!
                int destHost;
                do
                {
                    destHost = rand() % NUM_HOSTS;
                } while (destHost != currentEvent->host);
                Packet *packetReceived = new Packet;
                packet(&packetReceived, frameSize, destHost);
                hosts[currentEvent->host].push(packetReceived);
                hosts[currentEvent->host].backoff = (rand() % T) + 1;
            }
            // Processing the Departure Event
            else if (currentEvent->type == departure)
            {
                totalBytesTransmitted += currentEvent->bytes;
                channelBusy = false;
                if (currentEvent->isAck)
                {
                    hosts[currentEvent->host].pop();
                    if (hosts[currentEvent->host].head == NULL)
                    {
                        hosts[currentEvent->host].backoff = -1;
                    }
                    else
                    {
                        hosts[currentEvent->host].backoff = (rand() % T) + 1;
                    }
                }
                else
                {
                    // CREATING THE ACKNOWLEDGEMENT
                    Packet *tempAck = new Packet;
                    acknowledgementPacket(&tempAck, currentEvent->host);
                    hosts[currentEvent->host].pushFront(&tempAck);

                    if (hosts[currentEvent->host].backoff < 0)
                    {
                        hosts[currentEvent->host].backoff = (rand() % T) + 1;
                    }
                }
            }
            else if (currentEvent->type == backoff)
            {
                //create a new backoff event
                Event *event = new Event;
                backoffEvent(&event, currentTime);
                gel.insertEvent(&event);
                int departureHost = -1;
                bool resetBackOff = false;
                //we want to add queue delay time even if the channel is busy
                for(int i =0;i<NUM_HOSTS;i++)
                {
                    if (hosts[i].head != NULL && hosts[i].tail != NULL)
                            delay += intervalTime; //adding the queue delay time for every host
                }
                if (channelBusy == false)
                {
                    for (int i = 0; i < NUM_HOSTS; i++)
                    {
                        if (hosts[i].backoff != 0)
                            hosts[i].backoff--;
                        //NOTE: under the assumption that two events will not reach 0 at the same time
                        if (hosts[i].head != NULL && hosts[i].tail != NULL)
                        {
                            if (hosts[i].backoff == 0 && departureHost == -1)
                            {
                                departureHost = i;
                            }
                            else if (hosts[i].backoff == 0 && departureHost != -1)
                            {
                                hosts[i].backoff = rand() * T * 2;
                                hosts[departureHost].backoff = rand() * T * 2;
                                resetBackOff = true;
                            }
                        }
                    }

                    if (resetBackOff==false)
                    {
                        //MAKE DEPARTURE EVENT .... YAY
                        Packet *packet = hosts[departureHost].head;
                        int departureTime = packet->frameSize / channelRate;
                        bool isAck = packet->isAck;
                        Event *event = new Event;

                        departureEvent(&event, departureTime, currentTime, departureHost, isAck, packet->frameSize / 8);
                        gel.insertEvent(&event);
                        channelBusy = true;
                    }
                }
            }
            // gel.printGEL();
        }
        cout << "Lambda: " << arrivalRate << "\t"
             << "Delay: " << delay << "\t"
             << "Throughput: " << totalBytesTransmitted / (currentTime) << endl;
    }
    return 0;
}
