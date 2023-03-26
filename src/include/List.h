/*
 * ArrayList.h
 *
 * Implementation of an ArrayList that mirrors the
 * functions provided by the java ArrayList class.
 *
 *     Author: egaebel
 */
#ifndef ARRAYLIST_H_
#define ARRAYLIST_H_
#include <stddef.h>

template <class T>
class ArrayList {
 public:
  typedef T* iterator;

  //~Constructors-----------------------------------------------
  ArrayList();
  ArrayList(int theSize);
  ArrayList(const ArrayList<T>& list);
  ArrayList(const ArrayList<T>& list, int theCapacity);
  ~ArrayList();
  //~Methods---------------------------------------------------
  void add(const T& element);
  void add(const T& element, int index);
  bool addToSort(const T& element);
  void add(const ArrayList<T>& list, int index);
  void add(const ArrayList<T>& list);
  bool removeElement(const T& element);
  const T* remove(int index);
  void removeAll();
  void removeAll(int newCapacity);
  bool move(int index1, int index2);
  T* get(int index) const;
  iterator begin();
  iterator end();
  //~Extra Basic Functions-----------------------------------
  const ArrayList<T>& subList(int index1, int index2) const;
  T* getArray() const;
  T* getSubArray(int index1, int index2) const;
  //~Binary Search-------------------------------------------
  int binarySearch(const T& element) const;
  //~SORTS-----------------------------------
  void quickSort();
  void insertionSort();
  void mergeSort();
  // void shuffle();
  //~Information Getting methods
  unsigned int getSize() const;
  unsigned int getCapacity() const;
  // std::string toString() const;

 private:
  // The array that is the basis for this ArrayList.
  T* array;
  // The current number of elements in this ArrayList.
  int size;
  // The current capacity of the array that is backing this.
  int capacity;
  // Boolean value indicating whether this ArrayList has been sorted.
  bool sorted;

  //~Methods-----------------------//
  void reallocate();
  void reallocate(int newSize);
  //~SORTS-----------------------------------
  const T& choosePivot(int index1, int index2);
  void quickSort(int index1, int index2);
  int binarySearch(const T& element, int index1, int index2) const;
  void insertionSort(int index1, int index2);
  T* mergeSort(T* array, int theCapacity, int arraySize);
};

//~Class Initialization/Destruction/Resetting Functions---------------------
/**
 * Default constructor,
 * creates a 20 element ArrayList, of type T.
 */
template <class T>
ArrayList<T>::ArrayList() {
  array = new T[20];
  capacity = 20;
  size = 0;
}

/**
 * Creates an ArrayList of type T of size
 * theSize.
 *
 * @param theSize the size to initialize the array to.
 */
template <class T>
ArrayList<T>::ArrayList(int theSize) {
  array = new T[theSize];
  capacity = theSize;
  size = 0;
}

/**
 * Creates an ArrayList of type T that is twice the
 * size of the passed in ArrayList, and adds all elements
 * from the passed ArrayList<T> list, to this ArrayList.
 *
 * Runs in O(n) time, where n = the size of the passed list.
 *
 * @param list the ArrayList to use as a seed for this ArrayList.
 */
template <class T>
ArrayList<T>::ArrayList(const ArrayList<T>& list) {
  array = new T[list.getSize() * 2];
  capacity = list.getSize() * 2;
  size = list.getSize();

  for (int i = 0; i < list.getSize(); i++) {
    array[i] = *list.get(i);
  }
}

/**
 * Creates an ArrayList of type T that has a capacity equal to the passed
 * in theCapacity parameter. This ArrayList starts with the passed ArrayList.
 *
 * Note: If the passed in capacity is smaller than the size of the passed in
 *          ArrayList, then the capacity is set to twice the size of the
 *          passed ArrayList.
 *
 * Runs in O(n) time where n is the size of the passed list.
 *
 * @param list the ArrayList to use as a seed for this ArrayList.
 * @param theCapacity the capacity for this ArrayList.
 */
template <class T>
ArrayList<T>::ArrayList(const ArrayList<T>& list, int theCapacity) {
  if (theCapacity >= list.getSize()) {
    array = new T[theCapacity];
    capacity = theCapacity;
  } else {
    array = new T[list->getSize() * 2];
    capacity = list->getSize() * 2;
  }

  size = list->size;

  for (int i = 0; i < size; i++) {
    array[i] = list->get(i);
  }
}

/**
 * General destructor, deallocates the array.
 */
template <class T>
ArrayList<T>::~ArrayList() {
  delete[] array;
}

/**
 * Doubles the capacity of the ArrayList.
 *
 * Runs in O(n) time where n is the current size of the list.
 */
template <class T>
void ArrayList<T>::reallocate() {
  capacity *= 2;
  T* temp = new T[capacity];

  for (int i = 0; i < size; i++) {
    temp[i] = array[i];
  }

  delete[] array;

  array = temp;
}

/**
 * Creates a new Array that is of size, newSize.
 *
 * Runs in O(n) time where n is the current size of the list..
 *
 * @param newSize the new size to make this ArrayList.
 */
template <class T>
void ArrayList<T>::reallocate(int newSize) {
  capacity = newSize;
  T* temp = new T[newSize];

  for (int i = 0; i < size; i++) {
    temp[i] = array[i];

    delete[] array;

    array = temp;
  }
}

//~Basic Add/Remove etc Functions------------------------------------
/**
 * Adds the passed in element to the end of the ArrayList.
 *
 * Runs in O(n) in worst case, where reallocate is called.
 * O(1) for most cases.
 *
 * @param element the element to add to the array.
 */
template <class T>
void ArrayList<T>::add(const T& element) {
  if ((size - 1) == capacity) {
    reallocate();
  }

  array[size] = element;
  size++;

  sorted = false;
}

/**
 * Adds the passed in element to the specified index. Provided
 * that the index is valid.
 *
 * A valid index is: 0 <= index <= size
 *
 * Runs in O(n) where n is either the number of elements that must
 * be shifted to fit element in index, or the size of the array if
 * the array has to be reallocated.
 *
 * @param element the element to add to the ArrayList.
 * @param index the index to add the element to.
 */
template <class T>
void ArrayList<T>::add(const T& element, int index) {
  if (index >= 0 && index <= size) {
    // Reallocate if necessary.
    if (index >= capacity || size == capacity) {
      int multiplicand = (index >= capacity) ? index : size;
      reallocate(multiplicand * 2);
    }

    // Shift elements to the right.
    for (int i = size; i > index; i--) {
      array[i] = array[i - 1];
    }

    array[index] = element;
    size++;

    sorted = false;
  }
}

/**
 * Adds an ArrayList<T> to the end of this ArrayList.
 *
 * Runs in O(n) time, where n = the size of the passed list.
 *
 * @param list the ArrayList<T> to add to this one.
 */
template <class T>
void ArrayList<T>::add(const ArrayList<T>& list) {
  for (int i = 0; i < list.getSize(); i++) {
    add(list.get(i));
  }
}

/**
 * Adds an ArrayList<T> to this ArrayList<T> at the specified index.
 *
 * Note: If index is larger than the capacity of the ArrayList, the
 *          ArrayList is enlarged to a size that is two times the index plus
 *          the size of the arrayList.
 *          If you want a check on the index size, use getSize() to check.
 *
 * Runs in O(n) time where n is the size of the passed in list, or the number
 * of elements which are shifted in this list. Whichever one is larger.
 *
 * @param list the ArrayList<T> to add to this one.
 * @param index the index to add the passed in ArrayList<T> at.
 */
template <class T>
void ArrayList<T>::add(const ArrayList<T>& list, int index) {
  // If passed in list will overflow off of this one.
  if (index >= capacity || (index + list.getSize()) >= capacity) {
    reallocate((index + list.getSize()) * 2);
  }

  T* temp = new T[capacity + list.getSize()];

  // Loop through array, and passed list, adding them where they are
  // appropriate.
  for (int i = 0, j = 0, k = 0; i < size + list.getSize(); i++) {
    if (i >= index && i < index + list.getSize()) {
      temp[i] = *list.get(j);
      j++;
    } else {
      temp[i] = array[k];
      k++;
    }
  }

  delete[] array;
  array = temp;
  size += list.getSize();

  sorted = false;
}

/**
 * Takes in an element of type T and adds it to the correct point
 * in the ArrayList such that the sort is preserved.
 *
 * NOTE: If the ArrayList is NOT sorted, the element is not inserted,
 *          and false is returned.
 *
 * Runs in O(n) time where n is the number of elements that must be shifted to
 * accommodate the inserted element.
 *
 * @param element the element to insert.
 * @return true if ArrayList was sorted (meaning the element was inserted),
 *          false if the ArrayList was NOT sorted (the element was NOT inserted)
 */
template <class T>
bool ArrayList<T>::addToSort(const T& element) {
  if (sorted) {
    add(element, binarySearch(element));

    sorted = true;

    return true;
  }

  return false;
}

/**
 * Removes the passed in element from the ArrayList if
 * it's present. Returns a boolean value indicating
 * if the element was present in the array.
 *
 * Runs in O(n) time where n is the number of elements that must be shifted
 * to fill the hole.
 *
 * @param element the element to remove from the array.
 * @return value true if the element was present, false otherwise.
 */
template <class T>
bool ArrayList<T>::removeElement(const T& element) {
  int index = binarySearch(element);

  if (index >= 0 && index < size) {
    remove(index);

    return true;
  } else {
    return false;
  }
}

/**
 * Removes the element at the passed in index.
 *
 * Runs in O(n) time where n is the number of elements that must be shifted.
 *
 * @param index the index of the element to remove.
 * @return removed a pointer to the removed element, or NULL/0 if
 *          the index is not valid.
 */
template <class T>
const T* ArrayList<T>::remove(int index) {
  T* removed = NULL;

  if (index < size && index >= 0) {
    for (int i = index; i < size; i++) {
      array[i] = array[i + 1];
    }

    removed = array + index;
    size--;
  }

  return removed;
}

/**
 * Empties the ArrayList of all elements, and creates a new ArrayList
 * of the same capacity as it was.
 *
 * Runs in O(1) time.
 */
template <class T>
void ArrayList<T>::removeAll() {
  delete[] array;
  array = new T[capacity];

  sorted = false;
  size = 0;
}

/**
 * Empties the ArrayList of all elements and creates a new ArrayList
 * of the capacity denoted by newCapacity.
 *
 * Runs in O(1) time.
 *
 * @param newCapacity the new capacity for the ArrayList.
 */
template <class T>
void ArrayList<T>::removeAll(int newCapacity) {
  delete[] array;
  array = new T[newCapacity];

  sorted = false;
  capacity = newCapacity;
  size = 0;
}

/**
 * Moves the element at index1 to index2, and the element
 * at index2 to index1. Returns true if the indices are
 * valid, false otherwise.
 *
 * Runs in O(1) time.
 *
 * @param index1 the index of the first element to move.
 * @param index2 the index of the first element to move.
 * @return true if the indices are valid, false otherwise.
 */
template <class T>
bool ArrayList<T>::move(int index1, int index2) {
  if (index1 >= 0 && index2 >= 0 && index1 < size && index2 < size) {
    T temp = array[index1];
    array[index1] = array[index2];
    array[index2] = temp;

    sorted = false;

    return true;
  }

  return false;
}

/**
 * Gets the element stored at the passed in index.
 *
 * Runs in O(1) time.
 *
 * @param index the index of the element to get.
 * @return a pointer to the element stored at index, or NULL if the index isn't
 * valid.
 */
template <class T>
T* ArrayList<T>::get(int index) const {
  return (index >= 0 && index < size) ? (array + index) : NULL;
}

//~Size and Capacity Functions----------------------------
/**
 * Gets the size of this ArrayList.
 *
 * @return size the number of elements in this ArrayList.
 */
template <class T>
unsigned int ArrayList<T>::getSize() const {
  return size;
}

/**
 * Gets the capacity of this ArrayList.
 *
 * @return the capacity of the ArrayList.
 */
template <class T>
unsigned int ArrayList<T>::getCapacity() const {
  return capacity;
}

template <class T>
typename ArrayList<T>::iterator ArrayList<T>::begin() {
  return array;
}

template <class T>
typename ArrayList<T>::iterator ArrayList<T>::end() {
  return array + getSize();
}

//~Extra Basic Functions-----------------------------------
/**
 * Gets an ArrayList containing the elements stored at index1(inclusive)
 * to index2 (exclusive; meaning the element at index2 is not included in
 * the subList, so it is possible to get a subList, where index2 is equal
 * to size).
 *
 * E.g: To get from element 5 to the end of the subList,
 *       get(4, getSize());
 *
 * Runs in O(m) time, where m = index2 - index1.
 *
 * @param index1 the starting index for the subList (inclusive).
 * @param index2 the ending index for the subList (exclusive.
 * @return returnList the subList designated by index1 & index2.
 *          Returns NULL if either index isn't valid, or if
 *          index1 > index2.
 */
template <class T>
const ArrayList<T>& ArrayList<T>::subList(int index1, int index2) const {
  if (index1 >= 0 && index1 < size && index2 >= 0 && index2 <= size &&
      index1 < index2) {
    ArrayList<T> returnList = new ArrayList<T>(index2 - index1);

    // Loop through all elements, so there isn't just a reference
    // copy performed.
    for (int i = index1; i < index2; i++) {
      returnList.add(this->get(i));
    }

    return returnList;
  }

  return NULL;
}

/**
 * Returns a copy of the array that is the basis for
 * this ArrayList.
 *
 * Note when using this method the returned array must
 * be deleted by the user when they are done with it.
 *
 * Runs in O(n) time where n = size.
 *
 * @return a copy of the array that is the
 *          basis of this ArrayList.
 */
template <class T>
T* ArrayList<T>::getArray() const {
  T* arrayCopy = new T[size];

  for (int i = 0; i < size; i++) {
    arrayCopy[i] = array[i];
  }

  return arrayCopy;
}

/**
 * Gets a sub array of the array backing this ArrayList.
 *
 * Runs in O(m) time where m = index2 - index1.
 *
 * @param index1 the first index, that starts the subArray, inclusive.
 * @param index2 the ending index of the subArray, exclusive.
 * @return returns null if index1 > index2, otherwise returns
 *          a T pointer to the first element of the array.
 *          The array's size is, index2 - index1.
 */
template <class T>
T* ArrayList<T>::getSubArray(int index1, int index2) const {
  if (index1 >= 0 && index1 < size && index2 >= 0 && index2 <= size &&
      index1 < index2) {
    T* returnList = new T[index2 - index1];

    // Loop through all elements, so there isn't just a reference
    // copy performed.
    for (int i = index1; i < index2; i++) {
      returnList[i] = this->get(i);
    }

    return returnList;
  }

  return NULL;
}

//~Binary Search---------------------------------
/**
 * Helper binarySearch method that takes in an element, and passes on a
 * starting pivot. The pivot is the point at which to split
 * the array in half at. Returns the index of the passed in element, or -1 if
 * the element is not present, or if the array isn't sorted.
 *
 * Note: You must sort the array before you can search it.
 *
 * Runs in O(log(base 2)n) time, where n = size of array.
 *
 * @param element the element whose index is sought.
 * @return the index of the element, or -1 if the element isn't present.
 */
template <class T>
int ArrayList<T>::binarySearch(const T& element) const {
  if (sorted) {
    return binarySearch(element, 0, size - 1);
  } else {
    return -1;
  }
}

/**
 * Main binarySearch method, takes in an element and a pivot, which is the
 * point at which to split the array in half at. Returns the index of the
 * element, or -1 if the element is not present.
 *
 * Note: You must sort the array before you can search it.
 *
 * Runs in O(log(base 2)n) time, where n = size of array.
 *
 * @param element the element whose index is sought.
 * @param pivot the index to split .
 * @return the index of the element, or -1 if the element isn't present.
 */
template <class T>
int ArrayList<T>::binarySearch(const T& element, int index1, int index2) const {
  if (index1 < index2) {
    int pivot = (index1 + index2) / 2;

    if (element < array[pivot]) {
      return binarySearch(element, index1, pivot - 1);
    } else if (element > array[pivot]) {
      return binarySearch(element, pivot + 1, index2);
    } else {
      return pivot;
    }
  } else if (index1 == index2) {
    return index1;
  } else {
    return -1;
  }
}

//~SORTS-----------------------------------
/**
 * Runs quick sort on this ArrayList. Clean interface to the recursive,
 * option filled, quickSort.
 */
template <class T>
void ArrayList<T>::quickSort() {
  int index1 = 0;
  int index2 = size - 1;

  quickSort(index1, index2);

  sorted = true;
}

/**
 * Sorts the leftmost, rightmost and center elements, to determine
 * which is the best pivot. Then places the pivot in the  to last
 * slot of the array, to go along with quickSort logic. This ensures that the
 * pivot will always be the center element.
 *
 * Runs in O(1) time.
 *
 * @param index1 the low index.
 * @param index2 the high index.
 * @return the value of the pivot.
 */
template <class T>
const T& ArrayList<T>::choosePivot(int index1, int index2) {
  int center = (index1 + index2) / 2;

  // If center is smaller than leftmost.
  if (array[center] < array[index1]) {
    // Swap.
    move(index1, center);
  }
  // If the rightmost is smaller than the leftmost.
  if (array[index1] > array[index2]) {
    // Swap.
    move(index1, index2);
  }
  // If the center is larger than the rightmost.
  if (array[center] > array[index2]) {
    // Swap.
    move(center, index2);
  }

  // Swap pivot value with second to last member.
  move(center, index2 - 1);

  return array[index2 - 1];
}

/**
 * Runs quick sort on this ArrayList, using the passed in indices and the
 * passed in pivot. If the range denoted by index1 and index2 is <= 10
 * then insertion sort is run to improve efficiency.
 * Recursive.
 *
 * Runs in O(nlog(base 2)n) in all cases because the pivot point will never be
 * the lowest element all the time due to the choosePivot method's functioning.
 *
 * @param index1 the index on the left side of the pivot.
 * @param index2 the index on the right side of the pivot.
 */
template <class T>
void ArrayList<T>::quickSort(int index1, int index2) {
  // If the range is <= 10, run insertion sort, else quickSort as usual.
  if (index2 - index1 >= 10) {
    // Find the pivot to be used for this iteration of the sort.
    const T& pivot = choosePivot(index1, index2);

    // Initialize loop values.
    int i = index1, j = index2 - 1;

    // Loop until break.
    while (true) {
      // Loop up and down array for elements on the wrong side.
      while (array[++i] < pivot)
        ;
      while (array[--j] > pivot)
        ;

      if (i < j) {
        move(i, j);
      } else {
        break;
      }
    }

    // Swap i with pivot, so pivot is placed properly.
    move(i, index2 - 1);

    quickSort(index1, i - 1);
    quickSort(i + 1, index2);
  } else {
    insertionSort(index1, index2);
  }
}

/**
 * Runs merge sort on this ArrayList<T>. Interface function to the central,
 * recursive, merge sort function.
 *
 * Runs in O(nlogn) time. However it consumes extra memory.
 */
template <class T>
void ArrayList<T>::mergeSort() {
  T* temp = mergeSort(array, capacity, size);
  delete[] array;
  array = temp;
  sorted = true;
}

/**
 * Runs merge sort on the passed in array. Recursive.
 *
 * Runs in O(nlogn) time. However it consumes extra memory.
 *
 * @param array the array to sort.
 * @param arraySize the size of the array that is to be sorted.
 * @return the sorted array.
 */
template <class T>
T* ArrayList<T>::mergeSort(T* array, int theCapacity, int arraySize) {
  T* returnArray;

  // If the array is more than one element.
  if (arraySize > 1) {
    int size1 = arraySize / 2;
    int size2 = arraySize - size1;

    T* array1;
    T* array2;

    // Recurse.
    array1 = mergeSort(array, size1, size1);
    array2 = mergeSort(array + size1, size2, size2);

    // Allocate memory for return array.
    returnArray = new T[theCapacity];

    // Loop through all elements in returnArray.
    int i = 0, j = 0, k = 0;
    while (i < arraySize) {
      // Place the lesser of two elements in returnArray.
      if ((j < size1) && (k == size2 || array1[j] <= array2[k])) {
        returnArray[i] = array1[j];
        j++;
      } else {
        returnArray[i] = array2[k];
        k++;
      }

      i++;
    }

    // Free the memory allocated in the recursive calls.

    delete[] array1;
    delete[] array2;
    array1 = 0;
    array2 = 0;
  }
  // If one element is in the passed array.
  else {
    // Allocate memory for new array, and assign passed value to it.
    // This is done so delete can be called in the calling function.
    returnArray = new T[1];
    returnArray[0] = array[0];
  }

  return returnArray;
}

/**
 * Runs insertion sort on this ArrayList.
 *
 * Runs in O(n^2) where n = the size of the ArrayList.
 */
template <class T>
void ArrayList<T>::insertionSort() {
  insertionSort(0, size - 1);

  sorted = true;
}

/**
 * Runs insertion sort on a subset of this ArrayList.
 *
 * Runs O(n^2) time where n = index2 - index1.
 *
 * @param index1 the first bounding index.
 * @param index2 the the second bounding index.
 */
template <class T>
void ArrayList<T>::insertionSort(int index1, int index2) {
  if (index1 < index2 && index1 >= 0 && index1 < size && index2 >= 0 &&
      index2 < size) {
    // Temporary value used to hold the member being inserted into the
    // sorted portion.
    T element;

    // Loop through all elements.
    for (int i = index1; i <= index2; i++) {
      element = array[i];

      // Loop through all inserted elements.
      for (int j = i; j >= 0; j--) {
        // Place the element in its spot.
        if (j == 0 || array[j - 1] <= element) {
          array[j] = element;
          break;
        }
        // Shift to the right.
        else {
          array[j] = array[j - 1];
        }
      }
    }
  }
}

/**
 * Loops through all of the elements in the ArrayList
 * and at each index the element is swapped with the element
 * at a random index.
 *
 * Essentially shakes up the ArrayList's ordering.
 *
 * Runs in O(n) time where n is the size of the array.
 */
/* template <class T>
void ArrayList<T>::shuffle() {
  T temp;
  int random;

  // Loop through all elements in array.
  for (int i = 0; i < size; i++) {
    // Swap element at index i with
    // an element at a random number.
    random = rand() % size;

    temp = array[i];
    array[i] = array[random];
    array[random] = temp;
  }

  sorted = false;
} */

/**
 * Returns a formatted string object of all
 * of the elements in the ArrayList.
 *
 * Ex. (element1, element2).
 *
 * Runs O(n) time where n is the size of the array.
 *
 * @return a string object representing the
 *          elements in the ArrayList.
 */
/* template <class T>
std::string ArrayList<T>::toString() const {
  std::ostringstream streamOut;

  streamOut << "(";

  for (int i = 0; i < size; i++) {
    streamOut << array[i];

    if (i != (size - 1)) {
      streamOut << ", ";
    }
  }

  streamOut << ")\n";

  std::string returnString = streamOut.str();

  return returnString;
} */

#endif /* ARRAYLIST_H_ */