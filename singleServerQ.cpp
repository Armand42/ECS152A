// Single Server Queue
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
#define SUCCESS true
#define FAILURE false

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
    int srcHost;
    int destHost;
    eventType type;
    Event *next;
    Event *prev;
};

Event *arrivalEvent(double arrivalRate, double currentTime, int srcHost)
{
    Event *event;
    event->eventTime = negativeExponentiallyDistributedTime(arrivalRate) + currentTime;
    event->type = arrival;
    event->srcHost = srcHost;
    int destHost;
    do
    {
        destHost = rand() % NUM_HOSTS;
    } while (destHost != srcHost);
    event->destHost = destHost;
    event->next = event->prev = NULL;
    return event;
}

Event *backoffEvent(double currentTime)
{
    Event *event;
    event->eventTime = 0.1 + currentTime;
    event->type = backoff;
    event->srcHost = -1;
    event->destHost = -1;
    event->next = event->prev = NULL;
    return event;
}

Event *departureEvent(double serviceTime, double currentTime, int srcHost)
{
    Event *event;
    event->eventTime = serviceTime + currentTime;
    event->type = departure;
    event->srcHost = srcHost;
    int destHost;
    do
    {
        destHost = rand() % NUM_HOSTS;
    } while (destHost != srcHost);
    event->destHost = destHost;
    event->next = event->prev = NULL;
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

struct Packet
{
  public:
    int frameSize;
    int destHost;
    int srcHost;
};

struct Host
{
    queue<Packet> packetQueue;
    int backoff;
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
// Helper function to determine the current state of the packet
bool GEL::isTransmittingPacket()
{
    Event *current = head;
    while (current != NULL)
    {
        if (current->type == departure)
            return true;

        current = current->next;
    }
    return false;
}

// Helper function to print all Queue elements
void printQueue(queue<Packet *> q)
{

    cout << "Queue: ";
    while (!q.empty())
    {
        printf("%f ->", q.front()->serviceTime);
        q.pop();
    }
    cout << endl;
}

int main()
{
    bool channelBusy = false;
    double delay = 0;
    const double channelRate = 10000000; //Mbps
    const double SIFS = .00005;          //sec
    const double DIFS = .0001;           //sec
    const double backoffTime = 0.1;      //sec

    queue<Packet *> queue;
    cout << "***STATISTICS***\n";
    cout << endl;
    double arrivalRates[7] = {0.1, 0.2, 0.4, 0.5, 0.6, 0.8, 0.9};
    for (int i = 0; i < 7; i++)
    {

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
            Event *event = arrivalEvent(arrivalRate, currentTime, i);
            gel.insertEvent(&event);
        }

        Event *event = backoffEvent(currentTime);
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

                Event *newArrival = arrivalEvent(arrivalRate, currentTime, currentEvent->srcHost);
                gel.insertEvent(&newArrival);
                // Processing the Arrival Event
                // CREATING THE ACKNOWLEDGEMENT
                Packet acknowledgement = {ACKPACKETSIZE, currentEvent->destHost, currentEvent->srcHost};
                hosts[currentEvent->destHost].packetQueue.push(acknowledgement);
                Packet packetReceived = {frameSize, currentEvent->destHost, currentEvent->srcHost};
                hosts[currentEvent->destHost].packetQueue.push(packetReceived);
            }
            // Processing the Departure Event
            else if (currentEvent->type == departure)
            {
                totalNumberOfPackets += intervalTime * (queue.size() + 1);
                serverBusyTime += intervalTime;
                if (queue.size() > 0)
                {
                    Packet *departurePacket = queue.front();
                    queue.pop();
                    double serviceTime = departurePacket->serviceTime;

                    Event *departureEvent = new Event;
                    departureEvent->eventTime = currentTime + serviceTime;
                    departureEvent->type = departure;
                    departureEvent->next = departureEvent->prev = NULL;
                    gel.insertEvent(&departureEvent);
                }
            }
            else if (currentEvent->type == backoff)
            {
                //create a new backoff event
                Event *event = backoffEvent(currentTime);
                gel.insertEvent(&event);
                int departureHost = -1;
                for (int i = 0; i < NUM_HOSTS; i++)
                {
                    hosts[i].backoff--;
                    //NOTE: under the assumption that two events will not reach 0 at the same time
                    if(!hosts[i].packetQueue.empty())
                    {
                        delay+=intervalTime; //adding the queue delay time for every host
                        if (hosts[i].backoff == 0 )
                        {
                            departureHost = i;
                        }
                    }
                }

                if (departureHost != -1)
                {
                    //MAKE DEPARTURE EVENT .... YAY
                    Packet packet = hosts[departureHost].packetQueue.front();
                    int departureTime = packet.frameSize / channelRate;
                    Event *event = departureEvent(departureTime, currentTime, departureHost);
                    gel.insertEvent(&event);
                }
            }
        }

        printf("Utilization: %f \tMean Queue Length: %f \tNumber of Packets Dropped: %f \n", ((double)serverBusyTime) / currentTime, ((double)totalNumberOfPackets) / currentTime, droppedPackets);
    }

    return 0;
}
