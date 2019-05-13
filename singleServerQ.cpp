// Single Server Queue
// Yara Fawaz   912243010
// Armand Nasseri   912679383
// Jennifer Nguyen  914995612

#include <iostream>
#include <iomanip>
#include <queue>
#include <math.h>

#define MAXBUFFER 30
#define ARRIVAL 0
#define DEPARTURE 1
#define SUCCESS true
#define FAILURE false

using namespace std;
// Event Node
struct Event
{
  public:
    double eventTime;
    bool eventType;
    Event *next;
    Event *prev;
};

// Global Event list: Implemented as a double linked list
class GEL
{
  public:
    Event *head;
    Event *tail;
    GEL();
    bool insertEvent(Event *newEvent);
    Event *removeFirstEvent();
    void printGEL();
    bool isTransmittingPacket();
};

struct Packet
{
  public:
    double serviceTime;
    Packet *next;
};

GEL::GEL()
{
    head = tail = NULL;
}

// Insert an event at the proper location of the linked list
bool GEL::insertEvent(Event *newEvent)
{
    if (head == NULL && tail == NULL)
    {
        head = tail = newEvent;
        return SUCCESS;
    }
    Event *index = head;
    while (index != NULL && index->eventTime < newEvent->eventTime)
    {
        index = index->next;
    }
    if (index == head)
    {
        newEvent->next = head;
        head->prev = newEvent;
        head = newEvent;
    }
    else if (index == NULL)
    {
        newEvent->prev = tail;
        tail->next = newEvent;
        tail = newEvent;
    }
    else
    {
        newEvent->next = index;
        newEvent->prev = index->prev;
        index->prev->next = newEvent;
        index->prev = newEvent;
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

        printf("%s EventTime: %f -> ", current->eventType == ARRIVAL ? "Arrival" : "Departure", current->eventTime);
        current = current->next;
    }
}
// Helper function to determine the current state of the packet
bool GEL::isTransmittingPacket()
{
    Event *current = head;
    while (current != NULL)
    {
        if (current->eventType == DEPARTURE)
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
// Function provided by assignment
double negativeExponentiallyDistributedTime(double rate)
{
    double u;
    u = drand48();
    return ((-1 / rate) * log(1 - u));
}

int main()
{
    queue<Packet *> queue;
    cout <<"***STATISTICS***\n";
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
        Event event = {negativeExponentiallyDistributedTime(arrivalRate) + currentTime, ARRIVAL, NULL, NULL};
        gel.insertEvent(&event);

        for (int i = 0; i < 100000; i++)
        {
            Event *currentEvent = gel.removeFirstEvent();
            intervalTime = currentEvent->eventTime - currentTime;
            currentTime = currentEvent->eventTime;
            if (currentEvent->eventType == ARRIVAL)
            {
                // Generating the next arrival
                double arrival = negativeExponentiallyDistributedTime(arrivalRate), departure = negativeExponentiallyDistributedTime(serviceRate);

                Event newArrival = {arrival + currentTime, ARRIVAL, NULL, NULL};
                gel.insertEvent(&newArrival);
                Packet packet = {departure, NULL};

                // Processing the Arrival Event
                if (!gel.isTransmittingPacket())
                {
                    double serviceTime = packet.serviceTime;
                    Event departureEvent = {(currentTime + serviceTime), DEPARTURE, NULL, NULL};
                    gel.insertEvent(&departureEvent);
                }
                else
                {

                    if (queue.size() >= MAXBUFFER)
                    {
                        droppedPackets++;
                    }
                    else
                    {
                        queue.push(&packet);
                    }
                    totalNumberOfPackets += intervalTime * queue.size();
                    serverBusyTime += intervalTime;
                }
            }
            // Processing the Departure Event
            else
            {
                if (queue.size() != 0)
                {
                    Packet *departurePacket = queue.front();
                    queue.pop();
                    double serviceTime = departurePacket->serviceTime;

                    Event departureEvent = {currentTime + serviceTime, DEPARTURE, NULL, NULL};
                    gel.insertEvent(&departureEvent);

                    totalNumberOfPackets += intervalTime * queue.size();
                    serverBusyTime += intervalTime;
                }
            }
            //printQueue(queue);
        }
        
        cout << "ARRIVAL RATE: " << arrivalRate << "\t";
        printf("Utilization: %f \tMean Queue Length: %f \tNumber of Packets Dropped: %f \n", ((double)serverBusyTime) / currentTime, ((double)totalNumberOfPackets) / currentTime, droppedPackets);
        //cout << "TOTAL NUMBER OF PACKETS: " << totalNumberOfPackets << endl;
        //cout << endl;
        //cout << "TOTAL NUMBER OF PACKETS: " << totalNumberOfPackets << endl;
    }

    return 0;
}
