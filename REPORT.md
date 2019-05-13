# ECS 152 Project 1 Write Up

Yara Fawaz

Armand Nasseri

Jennifer Nguyen

## **Data Structures**

```c++
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

struct Event
{
  public:
    int eventTime;
    bool eventType;
    Event *next;
    Event *prev;
};
```

The `GEL` data structure is implemented as a doubly linked list. The `GEL` data structure includes a pointer to the head and tail of the linked list for ease of access. The `Event` data structure is a node in the `GEL`.

## **IMPLEMENTATION DETAILS**

### **Helper Functions**


### **Main**


## Plots


