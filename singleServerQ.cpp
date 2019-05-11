#include <iostream>
#include <iomanip>
#include <queue>
#include <math.h>

#define MAXBUFFER 100000
#define ARRIVAL 0
#define DEPARTURE 1
#define SUCCESS true
#define FAILURE false
#define LAMBDA 0.1 //rate
// How to increment time
//
using namespace std;

struct Event
{
  public:
    int eventTime; //time when event occurs
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
    bool insertAtTail(Event *newEvent);
    bool insertEvent(Event *newEvent);
    Event *removeFirstEvent();
};

struct Packet
{
  public:
    int serviceTime;
    Packet *next;
};

class FIFOQueue
{
    public:
    Packet *front;
    Packet *back;
    uint length;

    FIFOQueue();
    bool insertPacket(Packet *newPacket);
    Packet *removePacket();
    bool isQueueFull();
};

GEL::GEL()
{
    head = tail = NULL;
}

bool GEL::insertAtTail(Event *newEvent)
{
    if (head == NULL && tail == NULL)
    {
        head = tail = newEvent;
        return SUCCESS;
    }
    tail->next = newEvent;
    newEvent->prev = tail;
    tail = newEvent;
    return SUCCESS;
}

bool GEL::insertEvent(Event *newEvent)
{
    if (head == NULL && tail == NULL) 
    {
        head = tail = newEvent;
        return SUCCESS;
    }
    Event *index = tail;
    while ((index != NULL) && (index->eventTime > newEvent->eventTime))
    {
        index = index->prev;
    }
    if (index == NULL) {
        newEvent->next = head;
        head->prev = newEvent;
        head = newEvent;
    }
    else {
        newEvent->next = index->next;
        newEvent->prev = index;
        if (index->next != NULL) {
            newEvent->next->prev = newEvent;
        }
        index->next = newEvent;
    }

    return SUCCESS;
}

Event *GEL::removeFirstEvent()
{
    Event *first = head;
    if (head == tail) 
    {
        head = tail = NULL;
        return first;
    }

    head = first->next;
    return first;
}

FIFOQueue::FIFOQueue()
{
    front = back = NULL;
    length = 0;
}

bool FIFOQueue::insertPacket(Packet *newPacket)
{
    back->next = newPacket;
    back = newPacket;
    length++;
    return SUCCESS;
}

Packet *FIFOQueue::removePacket()
{
    if (front == NULL)
        return NULL;
    Packet *first = front;
    front = first->next;
    length--;
    return first;
}

bool FIFOQueue::isQueueFull()
{
    if (length >= MAXBUFFER)
        return true;
    return false;
}

double negativeExponentiallyDistributedTime(double rate) 
{
    double u;
    u = drand48();
    return ((-1/rate) * log(1-u));
}

int main()
{
    
    //Time and counters for statistics.
    int currentTime = 0, serverBusyTime = 0, totalNumberOfPackets = 0, droppedPackets = 0, intervalTime = 0; 
    //Rates
    double serviceRate = 1; //mu
    double arrivalRate = 0.1; //lamda
    //Data Structure
    GEL gel = GEL();
    
    FIFOQueue queue = FIFOQueue();
    Event event = {negativeExponentiallyDistributedTime(arrivalRate) + currentTime, ARRIVAL, NULL, NULL};

    gel.insertEvent(&event);

    for (int i = 0; i < 100000; i++) {
        Event *currentEvent = gel.removeFirstEvent();
        intervalTime = currentEvent->eventTime - currentTime;
        currentTime = currentEvent->eventTime;
        
        if (currentEvent->eventType == ARRIVAL) {
            // Generating the next arrival
            Event newArrival = {negativeExponentiallyDistributedTime(arrivalRate) + currentTime, ARRIVAL, NULL, NULL};
            gel.insertEvent(&newArrival);
            Packet packet = {negativeExponentiallyDistributedTime(serviceRate), NULL};
            // Processing the Arrival Event
            if (queue.length == 0) 
            {
                int serviceTime = packet.serviceTime;
                Event departureEvent = {currentTime + serviceTime, DEPARTURE, NULL, NULL};
                gel.insertEvent(&departureEvent);
            }
            else {
                if (queue.isQueueFull())  
                {
                    droppedPackets++;  
                } 
                else 
                {
                    queue.insertPacket(&packet);
                    queue.length++;
                }
                totalNumberOfPackets += intervalTime * queue.length;
                serverBusyTime+= intervalTime;
            }
        }
        // else departure event
        else 
        {
            if (queue.length != 0)
            {
                Packet *departurePacket = queue.removePacket();
                int serviceTime = departurePacket->serviceTime;
                Event departureEvent = {currentTime + serviceTime, DEPARTURE, NULL, NULL};
                gel.insertEvent(&departureEvent);
                queue.length--;
                totalNumberOfPackets += intervalTime * queue.length;
                serverBusyTime+= intervalTime;
            }
        }
       
    }
    printf("Utilization: %f \nMean Queue Length: %f \nNumber of Packets Dropped: %d ", ((double)serverBusyTime)/currentTime, ((double)totalNumberOfPackets)/currentTime, droppedPackets);
    


    return 0;
}