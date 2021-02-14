#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>
#include "UniformRandom.h"

using namespace std;

template <typename AnyType>
class CuckooHashFamily
{
    public:
    size_t hash( const AnyType & x, int which ) const;
    int getNumberOfFunctions( );
    void generateNewFunctions( );
    };

    template <typename AnyType, typename HashFamily>
    class CuckooHashTable
    {
        public:
        explicit CuckooHashTable( int size = 101 );
        void makeEmpty( );
        bool contains( const AnyType& x ) const;
        bool remove( const AnyType & x );
        int insert( const AnyType & x );
        int insert( AnyType && x );
    
    private:
    struct HashEntry
    {
        AnyType element;
        bool isActive;
        HashEntry( const AnyType& e = AnyType( ), bool a = false )
        : element{ e }, isActive{ a } { }
        HashEntry( AnyType && e, bool a = false )
        : element{ std::move( e ) }, isActive{ a } { }
        };
        
        bool insertHelper1( const AnyType & xx );
        bool insertHelper1( AnyType && xx );
        bool isActive( int currentPos ) const;

        size_t myhash( const AnyType & x, int which ) const;
        int findPos( const AnyType& x ) const;
        void expand( );
        void rehash( );
        void rehash( int newSize );
        constexpr static const double MAX_LOAD = 0.40;
        static const int ALLOWED_REHASHES = 5;

        vector<HashEntry> array;
        int currentSize;
        int numHashFunctions;
        int rehashes;
        UniformRandom r;
        HashFamily hashFunctions;
        };