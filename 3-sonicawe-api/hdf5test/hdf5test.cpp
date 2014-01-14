#include <iostream>
#include "adapters/hdf5.h"
#include "signal/buffer.h"
#include "demangle.h"

using namespace std;
using namespace Signal;
using namespace Adapters;

int main(int, char *[])
{
    try
    {
        pBuffer data_;
        data_.reset ( new Buffer(Interval(0, 10), 1, 1));
        float* p = data_->getChannel (0)->waveform_data ()->getCpuMemory ();
        for (int i=0; i<data_->number_of_samples (); ++i)
            p[i] = i*10;


        const char* filename = "hdf5test.h5";
        Hdf5Buffer::saveBuffer ( filename, *data_, 0);


        double dummy1;
        Signal::pBuffer dummy2;


        pBuffer loadedwrittendata = Hdf5Buffer::loadBuffer ( filename, &dummy1, &dummy2);
        if( *loadedwrittendata != *data_ )
            throw runtime_error("Failed read");


        pBuffer loadedoctaveoutput = Hdf5Buffer::loadBuffer ( "octaveoutput.h5", &dummy1, &dummy2);
        if ( *loadedoctaveoutput != *data_ )
            throw runtime_error("Failed octave read");


        cout << "OK" << endl;
        return 0;
    }
    catch (const exception& x)
    {
        cout << "Error: " << vartype(x) << endl
                  << "Details: " << x.what() << endl;
        return -1;
    }
}
