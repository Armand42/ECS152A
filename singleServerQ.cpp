#include <iostream>
#include <iomanip>
#include <queue>

#define MAXBUFFER 100000
#define ARRIVAL 0
#define DEPARTURE 1
#define SUCCESS true
#define FAILURE false
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
    Event *head;
    Event *tail;
    GEL();
    bool insertAtTail(Event *newEvent);
    bool insertEvent(Event *newEvent, Event *prevEvent);
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

bool GEL::insertEvent(Event *newEvent, Event *prevEvent)
{
    newEvent->next = prevEvent->next;
    newEvent->prev = prevEvent;
    if (prevEvent != tail)
        prevEvent->next->prev = newEvent;
    prevEvent->next = newEvent;
    return SUCCESS;
}

Event *GEL::removeFirstEvent()
{
    Event *first = head;
    if (head == tail)
        head = tail = NULL;
    return first;
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

int main()
{

    GEL gel();
    FIFOQueue queue();


    return 0;
}