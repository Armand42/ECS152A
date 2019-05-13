#include <iostream>
#include <iomanip>
#include <queue>
#include <math.h>

#define MAXBUFFER -1
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
    double eventTime; //time when event occurs
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
    void printGEL();
    bool isTransmittingPacket();
};

struct Packet
{
  public:
    double serviceTime;
    Packet *next;
};
/*
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
    void printQueue();
};
*/


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
    Event *index = head;
    while(index != NULL && index->eventTime < newEvent->eventTime)
    {
        index = index->next;
    }
    if(index == head)
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

void GEL::printGEL()
{
    Event *current = head;
    cout<<"\nGEL: ";
    int i = 0;
    while(current!=NULL)
    {

        printf("%s EventTime: %f -> ",current->eventType ==ARRIVAL?"Arrival":"Departure",current->eventTime);
        current = current->next;
    }
}

bool GEL::isTransmittingPacket()
{
    Event *current = head;
    while(current != NULL)
    {
        if(current->eventType == DEPARTURE)
            return true;

        current = current->next;
    }
    return false;
}

/*FIFOQueue::FIFOQueue()
{
    front = back = NULL;
    length = 0;
}

bool FIFOQueue::insertPacket(Packet *newPacket)
{
    // first packet in the list
    if(front == NULL)
        front = back = newPacket;
    // other packets
    else{
        back->next = newPacket;
        back = newPacket;
    }
    length++;
    // cout << "LENGTH: " << length << endl;
    return SUCCESS;
}

Packet *FIFOQueue::removePacket()
{
    if (front == NULL)
        return NULL;
    Packet *first = front;
    front = first->next;
    length--;
     cout << "REMOVE LENGTH: " << length << endl;
    return first;
}

bool FIFOQueue::isQueueFull()
{
    if(MAXBUFFER == -1)
        return false;
    if (length >= MAXBUFFER)
        return true;
    return false;
}

void FIFOQueue::printQueue()
{
    Packet *current = front;
}
*/
void printQueue(queue<Packet*> q)
{
	//printing content of queue 
    cout<<"Queue: ";
	while (!q.empty()){
		printf("%f ->",q.front()->serviceTime);
		q.pop();
	}
	cout<<endl;
}
double negativeExponentiallyDistributedTime(double rate) 
{
    double u;
    u = drand48();
    return ((-1/rate) * log(1-u));
}

int main()
{
	queue<Packet*> queue;
    cout << "STATISTICS\n";
    cout << endl;
    double arrivalRates[7] = {0.1, 0.2, 0.4, 0.5, 0.6, 0.8, 0.9};
    for(int i = 0; i < 7; i++)
    {
        
        //Time and counters for statistics.
        double currentTime = 0, serverBusyTime = 0, totalNumberOfPackets = 0, droppedPackets = 0, intervalTime = 0; 
        //Rates
        double serviceRate = 1; //mu
        double arrivalRate = arrivalRates[i]; //lamda

        //Data Structure
        GEL gel = GEL();
        Event event = {negativeExponentiallyDistributedTime(arrivalRate) + currentTime, ARRIVAL, NULL, NULL};
        gel.insertEvent(&event);
        
        for (int i = 0; i < 100000; i++) {
        //    gel.printGEL();

            Event *currentEvent = gel.removeFirstEvent();
            intervalTime =  currentEvent-> eventTime - currentTime;
            // cout<<"\t \t interval time: "<< intervalTime << "\t current time: "<< currentTime <<endl;
            currentTime = currentEvent->eventTime;
            if (currentEvent->eventType == ARRIVAL) {
                // Generating the next arrival
                double arrival =negativeExponentiallyDistributedTime(arrivalRate), departure =  negativeExponentiallyDistributedTime(serviceRate);
               // printf("\tarrival: %f        \t departure: %f \n", arrival, departure);
                Event newArrival = { arrival + currentTime, ARRIVAL, NULL, NULL};
                gel.insertEvent(&newArrival);
                Packet packet = {departure, NULL};
                
                // Processing the Arrival Event
                if (!gel.isTransmittingPacket()) 
                {
                    double serviceTime = packet.serviceTime;
                    Event departureEvent = {(currentTime + serviceTime), DEPARTURE, NULL, NULL};
                    gel.insertEvent(&departureEvent);

                }
                else {
           //         cout<<"INSERT PACKET\n";
                    if (queue.size() >= MAXBUFFER)  
                    {
                        droppedPackets++;  
                    } 
                    else 
                    {
                        queue.push(&packet);
                    
                    }
                    totalNumberOfPackets += intervalTime * queue.size();
                    serverBusyTime+= intervalTime;
                }
            }
            // else departure event
            else 
            {
                if (queue.size() != 0)
                {
                    Packet *departurePacket = queue.front();
		    queue.pop();
                    double serviceTime = departurePacket->serviceTime;
                    // printf(f"interval time %f\n", intervalTime);
		    Event departureEvent = {currentTime + serviceTime, DEPARTURE, NULL, NULL};
                    gel.insertEvent(&departureEvent);
                    
                    totalNumberOfPackets += intervalTime * queue.size();
                    serverBusyTime+= intervalTime;
                }
            }
            //printQueue(queue);
        
        }
        // cout << endl;
        // cout << "Arrival Rate " << arrivalRate << endl;
        // cout << "UTILIZATION: "
        cout << "ARRIVAL RATE: " << arrivalRate<< "\t";
        printf("Utilization: %f \tMean Queue Length: %f \tNumber of Packets Dropped: %f \n", ((double)serverBusyTime)/currentTime, ((double)totalNumberOfPackets)/currentTime, droppedPackets);
        //cout << "TOTAL NUMBER OF PACKETS: " << totalNumberOfPackets << endl;
        //cout << endl;
        //cout << "TOTAL NUMBER OF PACKETS: " << totalNumberOfPackets << endl;
        
    }    

    return 0;
}
