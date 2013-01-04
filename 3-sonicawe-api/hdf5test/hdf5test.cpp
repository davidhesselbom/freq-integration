#include <iostream>
#include "adapters/hdf5.h"
#include "signal/buffer.h"
#include "tfr/stft.h"

using namespace std;
using namespace Signal;
using namespace Adapters;
using namespace Tfr;

template<typename T>
bool operator!=(DataStorage<T>& A, DataStorage<T>& B)
{
    if (A.size () != B.size ())
        return true;

    T* a = A.getCpuMemory ();
    T* b = B.getCpuMemory ();
    return 0 != memcmp(a, b, A.numberOfBytes ());
}


int main(int, char *[])
{
    try
    {
        pBuffer data_;
        data_.reset ( new Buffer(Interval(0, 10), 1, 2));
        for (unsigned c=0; c<data_->number_of_channels (); ++c)
        {
            float* p = data_->getChannel (c)->waveform_data ()->getCpuMemory ();
            for (int i=0; i<data_->number_of_samples (); ++i)
                p[i] = i*10 + c*1000;
        }


        const char* filename = "hdf5test.h5";
        Hdf5Buffer::saveBuffer ( filename, *data_, 0);


        double dummy1;
        Signal::pBuffer dummy2;


        pBuffer loadedwrittendata = Hdf5Buffer::loadBuffer ( filename, &dummy1, &dummy2);
        if( *loadedwrittendata != *data_ )
            throw runtime_error("Failed read");


        pBuffer loadedoctaveoutput = Hdf5Buffer::loadBuffer ( "octaveoutput.h5", &dummy1, &dummy2);
        if ( *loadedoctaveoutput != *data_)
            throw runtime_error("Failed octave read");


        pMonoBuffer data2( new MonoBuffer(Interval(0,8), 1));
        *data2 |= *data_->getChannel (0);
        pChunk chunk = Fft(true)(data2);

        const char* chunkname = "hdf5testchunk.h5";
        Hdf5Chunk::saveChunk (chunkname, *chunk);


        pChunk loadedchunk = Hdf5Chunk::loadChunk ( chunkname );
        if (*loadedchunk->transform_data != *chunk->transform_data)
            throw runtime_error("Failed chunk read");


        pChunk loadedoctavechunk = Hdf5Chunk::loadChunk ( "octavechunk.h5" );
        // note that this is an exact comparsion of float data.
        if (*loadedoctavechunk->transform_data != *chunk->transform_data)
            throw runtime_error("Failed octave chunk read");


        cout << "OK" << endl;
        return 0;
    }
    catch (const exception& x)
    {
        cout << "Error: " << vartype(x) << endl
                  << "Details: " << x.what() << endl;
        return 1;
    }
}
