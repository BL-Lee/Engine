struct LinkedListNode
{
  LinkedListNode* next;
  LinkedListNode* prev;
  void* data;
};
struct LinkedList
{
  LinkedListNode* head;
  LinkedListNode* tail;
  u32 size;
  u32 dataSize;
  
  void* operator[](int index);
};

LinkedList* initLinkedList(u32 dataSize)
{
  LinkedList* list = (LinkedList*)malloc(sizeof(LinkedList));
  list->size = 0;
  list->dataSize = dataSize;
  list->head = NULL;
  list->tail = NULL;
  return list;
}

void* removeLinkedList(u32 index, LinkedList* list)
{
  if (list->size == 0)
    {
      return NULL;
    }
  LinkedListNode* stepper = list->head;
  for (int i = 1; i < index - 1; i++)
    {
      stepper = stepper->next;
    }
  LinkedListNode* removed = stepper->next;
  stepper->next->next->prev = stepper;
  stepper->next = stepper->next->next;
  void* data = removed->data;
  free(removed);
  return data;
}

void addLinkedList(void* data, LinkedList* list)
{
  LinkedListNode* node = (LinkedListNode*)malloc(sizeof(LinkedListNode));
  node->data = malloc(list->dataSize);
  memcpy(node->data, data, list->dataSize);
  if (list->size == 0)
    {
      list->head = node;
      list->tail = node;
    }
  else if (list->size == 1)
    {
      list->tail = node;
      node->prev = list->head;
      list->head->next = node;
    }
  else
    {
      list->tail->next = node;
      node->prev = list->tail;
      list->tail = node;	
    }
}

void* LinkedList::operator[](int index)
{
  if (size == 0) { return NULL; }
  if (index <= size / 2)
    {
      LinkedListNode* stepper = head;
      for (int i = 1; i < index; i++)
	{
	  stepper = stepper->next;
	}
      return stepper->data;
    }
  else
    {
      LinkedListNode* stepper = tail;
      for (int i = size - 1; i > index; i--)
	{
	  stepper = stepper->prev;
	}
      return stepper->data;
    }
}
