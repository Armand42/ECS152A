# ECS 152 Project 1 Write Up

Yara Fawaz 912243010

Armand Nasseri 912679383

Jennifer Nguyen 914995612

## **DATA STRUCTURES**

```c++
#define ARRIVAL 0
#define DEPARTURE 1

----------------------

struct Event
{
  public:
    double eventTime;
    bool eventType;
    Event *next;
    Event *prev;
};

class GEL
{
    public:
    Event *head;
    Event *tail;
    GEL();
    ...
};
```

The `GEL` data structure is implemented as a doubly linked list. The `GEL` data structure includes a pointer to the head and tail of the linked list for ease of access. The `Event` data structure is a node in the `GEL`. In the `Event` data structure, we include a double for the event time and a boolean that indicates the event type based on our macros (`ARRIVAL` & `DEPARTURE`).

```c++
struct Packet
{
  public:
    double serviceTime;
    Packet *next;
};
int main()
{
    queue<Packet*> queue;
    ...
}

```

The FIFO queue was implemented using the standard C++ library.  The queue is made up of `Packets`. A `Packet` has a structure analogous to a singly linked list and it contains a double that holds the service time for the specified packet.

## **IMPLEMENTATION DETAILS**

### **Helper Functions**

```c++
class GEL
{
    public:
    ...
    GEL();
    bool insertEvent(Event *newEvent);
    Event *removeFirstEvent();
    void printGEL();
    bool isTransmittingPacket();
};

void printQueue(queue<Packet*> q);
double negativeExponentiallyDistributedTime(double rate);

```

`GEL()` is the constructor for the GEL. It initializes the `head` and `tail` pointer to `NULL`.

`insertEvent()` is a function that will take the `newEvent` and insert the event chronologically into the GEL. It returns `SUCCESS` upon successful insertion.

`removeFirstEvent()` is the function that removes the first event in the GEL and returns a pointer to the removed event.

`printGEL()` is a function that will print out the GEL to the terminal. This function is used for debugging.

`isTransmittingPacket()` is a function that checks if the transmitter is busy transmitting another packet. We know the transmitter is busy if there is a departure event in the GEL.

`printQueue()` is a function that will print out the queue to the terminal. This function is used for debugging.

`negativeExponentiallyDistributedTime()` is a function given to us in the lab manual. This function is used to get the randomized distributed time based on `serviceRate` and `arrivalRate`.

### **Main**

The main logic was implemented akin to the logic given in the lab manual. We wrote a for loop to run each of the `arrivalRates` and we manually changed the `MAXBUFFER` for part 3 of the experiments.

## **EXPERIMENTS**
