// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#ifndef SMDATASTRUTCTURES_H
#define SMDATASTRUTCTURES_H

// SimMedTK includes
#include "Config.h"

template<typename T>
class smIndiceArrayIter;

/// \brief  inidice array for fast access to index
template<typename T>
class smIndiceArray
{
protected:
    /// \brief  storage
    T *storage;
    /// \brief  check if the slot is empty
    bool *isEmpty;
    /// \brief  indices
    int *indices;
    /// \brief  number of elements
    int nbrElements;
    /// \brief  maximum storage
    int maxStorage;
public:
    /// \brief  destructor
    ~smIndiceArray();
    /// \brief constructor; gets maximum number of elements
    inline smIndiceArray(int p_maxStorage);
    /// \brief  add item
    inline int add(T p_item);
    /// \brief check if the item exists, if not add
    inline int checkAndAdd(T p_item);
    /// \brief remove the element from array
    inline bool remove(int p_itemIndex);

    /// \brief replace the entry in p_index slot with p_item
    inline bool replace(int p_index, T &p_item);
    /// \brief get element by reference
    inline T& getByRef(int p_index);
    /// \brief  get element by safe. If the element doesn't exist it will return false
    inline bool getByRefSafe(int p_index, T&p_item);
    /// \brief get element by reference with string
    inline T& getByRef(std::string p_string);
    /// \brief for iterative access
    friend smIndiceArrayIter<T>;
    /// \brief print the elements
    inline void print() const;
    /// \brief operators
    inline T& operator[](int p_index);

    inline int size();
    /// \brief copy from another p_array
    inline bool copy(smIndiceArray &p_array);
};

/// \brief  iteration
template<class T>
class smIndiceArrayIter
{
    int index;
    smIndiceArray<T> *arrayPtr;
public:
    /// \brief constructor that require index array
    smIndiceArrayIter(smIndiceArray<T> *p_array);
    /// \brief  operators for accessing and iteration
    T& operator[](int p_index);

    int& operator++(); //prefix;

    int operator++(const int);  //postfix;

    int& operator--(); //prefix;

    int operator--(const int);  //postfix;

    inline int begin();

    inline int end();
};
/// \brief hash bucket size
#define SIMMEDTK_HASHBUCKET_SIZE    10
/// \brief a hash entry; bucket
template <class T>
struct smEntryList
{
public:
    smEntryList();

    T ID[SIMMEDTK_HASHBUCKET_SIZE];
    unsigned int totalEntries;
    smEntryList* p_entry;
};
/// \brief hash iterator
template <class T>
struct smHashIterator
{
    /// \brief  default initializaiton
    smHashIterator();
    /// \brief  to clone the has iterator
    inline void clone(smHashIterator<T> &p_iterator);

    template<typename K>
    inline void clone(smHashIterator<K> &p_iterator);
    /// \brief reseting the index for bucket iteration
    inline void resetBucketIteration();

    smEntryList<T> *iterator;
    smEntryList<T> *bucketStart;
    int tableIndex;
    int currentIndex;

    template <class K>
    friend std::ostream &operator<<(std::ostream &out, smHashIterator<K> &p);
};
/// \brief hash return codes
enum SIMMEDTK_HASHRETURN_CODES
{
    SIMMEDTK_HASH_ENTRYREMOVED,
    SIMMEDTK_HASH_ENTRYALREADYEXISTS,
    SIMMEDTK_HASH_ENTRYOTEXIST,
    SIMMEDTK_HASH_NOTBUCKETS,
    SIMMEDTK_HASH_ENTRYINSERTED,
    SIMMEDTK_HASH_ALLOCATED_INSERTED,
    SIMMEDTK_HASH_SUCCESS
};
/// \brief hash implementation. Table->
template <class T>
class smHash
{
    /// \brief primitive ids that is stored with entry list
    smEntryList<T> *primitiveIDs;
    /// \brief  current table index
    long int currentTableIndex;
    /// \brief current entry index in the bucket
    long int currentEntryIndex;
    /// \brief  current bucket pointer
    smEntryList<T> *currentIterationBucket;
public:
    /// \brief  number of pritimives
    long int num_prim;
    /// \brief  table size
    long int tableSize;
private:
    /// \brief entries in the bucket
    inline void moveEntriesInBucket(smEntryList<T> &p_Bucket, int p_entryIndex);
    /// \brief check if the p_prim exists in the bucket
    inline bool checkIdentical(smEntryList<T> &p_entry, T p_prim);
    /// \brief find the bucket entry and update it
    inline bool findandUpdateEntry(smEntryList<T> &p_startEntry, T &p_prim);
    /// \brief clear buckets
    inline void clearBuckets(smEntryList<T> &p_startEntry);

public:
    /// \brief initializes the table; entry
    smHash(int p_tableSize);
    /// \brief  insert the entry; indicate as p_triangle. It can be any type not only a triangle
    /// hashindex is the has number generated for the entry
    inline SIMMEDTK_HASHRETURN_CODES insert(T p_triangle, unsigned int hashIndex);
    /// \brief check if there is an entry and index
    inline SIMMEDTK_HASHRETURN_CODES checkAndInsert(T p_triangle, unsigned int hashIndex);
    /// \brief starts the iteration. reset the indices
    inline void startIteration();
    /// \brief  go to next table index
    inline bool next(smHashIterator<T> &p_iterator);
    /// \brief  iterate over the next bucket item
    inline bool nextBucketItem(smHashIterator<T> &p_iterator, T &p_prim);
    /// \brief proceed to next entry and return the element. it the bucket ends it will go to next table index
    inline bool next(T &p_prim);
    /// \brief next element by getting reference to it
    inline bool nextByRef(T **p_prim);
    /// \brief  print all the content
    inline void printContent();
    /// \brief  clear all table, including the elements
    void clearAll();
};
/// \brief sliding  storage type
enum smStorageSlideType
{
    SIMMEDTK_STORAGESLIDING_FRONTFIRST,
    SIMMEDTK_STORAGESLIDING_LASTFIRST
};
/// \brief sliding storage window
template<typename T>
class smStorageSlidingWindow
{
private:
    /// \brief  type
    smStorageSlideType strorageType;
public:
    /// \brief  data
    T *data;
    /// \brief window size
    unsigned int windowSize;
    /// \brief constructor
    ~smStorageSlidingWindow();
    /// \brief siding window storage. constructor  gets window size, type
    smStorageSlidingWindow(int p_windowSize = 10, smStorageSlideType p_type = SIMMEDTK_STORAGESLIDING_LASTFIRST);
    /// \brief storage type
    inline smStorageSlideType getStorageType();
    /// \brief  resize storage
    inline void resize(int p_newSize);
    /// \brief add value
    inline void add(T p_value);
    /// \brief  zero all the entries
    inline void zeroed();
    /// \brief  print all the elements
    void print() const;
};

#include "DataStructures.hpp"

#endif
