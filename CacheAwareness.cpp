#include<pthread.h>
#include<iostream>
#include<stdlib.h>
#include<chrono>
#include<vector>
#include<algorithm>

using namespace std;

// Number of processors
int const numProc = 32;
// First power of 2 over a billion
int const n = ( 1 << 31 ) - 1; 

int *data;
long long numOdd[ numProc ];

// Timers
vector< std::chrono::duration< double, milli > > procTimes( numProc );

// Threading infrastructure
pthread_barrier_t barrier;
pthread_t TID[ numProc ];

void* multiThreadedPass( void * arg );

bool verifyOutput() {
    for ( int i = 0; i < n; ++i ) {
        if ( data[ i ] ) return false;
    }
    return true;
}

void sequentialPassTiming() {
    auto start = std::chrono::high_resolution_clock::now();
    for ( int i = 0; i < n; ++i ) {
        if ( data[i] % 2 ) numOdd[0]++;
        data[i] = 0;
    }
    auto end = std::chrono::high_resolution_clock::now();
    procTimes[0] = end - start;
    cout << "Sequential timing: " << procTimes[0].count() << endl;
}

void generateRandomData() {
    srandom( 314159 ); // I like pi :)
    data = ( int * ) malloc( sizeof( int ) * n );
    for ( int i = 0; i < n; ++i ) {
        data[i] = random();
    }
}

int main() {
    generateRandomData();

    if ( numProc == 1 ) {
        sequentialPassTiming();
        bool verified = verifyOutput();
        if ( verified ) {
            cout << "Number of odd elements: " << numOdd[ 0 ] << endl;
        } else {
            cout << "Bad implementation, not zeroed out" << endl;
        }
        return 0;
    }

    pthread_setconcurrency( numProc );

    int shortID[ numProc ];
    pthread_barrier_init( &barrier, NULL, numProc );

    // Get the threads going.
    for ( int i = 1; i < numProc; ++i ) {
        shortID[ i ] = i;
        pthread_create( &TID[ i ], NULL, multiThreadedPass, &shortID[ i ] );
    }

    shortID [ 0 ] = 0;
    multiThreadedPass( ( void * ) &( shortID[ 0 ] ) );

    pthread_barrier_destroy( &barrier );

    bool verified = verifyOutput();
    if ( verified ) {
        cout << ( max_element( procTimes.begin(), procTimes.end() ) )->count()
            << " -> [ ";
        for ( int i = 0; i < numProc; ++i ) {
            cout << procTimes[ i ].count() << ", ";
        }
        cout << "] " << endl;
        int oddCount = 0;
        for ( int i = 0; i < numProc; ++i ) {
            oddCount += numOdd[ i ];
        }
        cout << "Number of odd elements: " << oddCount << endl;
    } else {
        cout << "Bad implementation, not zeroed out" << endl;
    }

    return 0;
}

void * multiThreadedPass( void * arg ) {
    int tid = *( int * ) arg;

    // Wait for all threads to get here to get the example working as intended.
    pthread_barrier_wait( &barrier );

    auto startTimer = std::chrono::high_resolution_clock::now();

    for ( unsigned int i = tid; i < n; i += numProc ) {
        if ( data[ i ] % 2 ) numOdd[ tid ]++;
        data[ i ] = 0;
    }

    // int start = ( n / numProc ) * tid;
    // int end = tid == numProc - 1 ? n : ( ( n / numProc ) * ( tid + 1 ) );

    // for ( unsigned int i = start; i < end; i++ ) {
    //     if ( data[ i ] % 2 ) numOdd[ tid ]++;
    //     data[ i ] = 0;
    // }
    
    // int oddCount = 0;
    // for ( unsigned int i = tid; i < n; i += numProc ) {
    //     if ( data[ i ] % 2 ) oddCount++;
    //     data[ i ] = 0;
    // }
    // numOdd[ tid ] = oddCount;

    // int start = ( n / numProc ) * tid;
    // int end = tid == numProc - 1 ? n : ( ( n / numProc ) * ( tid + 1 ) );
    // int oddCount = 0;
    // for ( unsigned int i = start; i < end; i++ ) {
    //     if ( data[ i ] % 2 ) oddCount++;
    //     data[ i ] = 0;
    // }
    // numOdd[ tid ] = oddCount;


    auto endTimer = std::chrono::high_resolution_clock::now();
    procTimes[ tid ] = endTimer - startTimer;
    pthread_barrier_wait( &barrier );
    return NULL;
}
