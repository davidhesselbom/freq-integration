if ~exist("OCTAVE_VERSION","builtin")
    error('Supposed to be run from Octave');
end

inputfile = 'hdf5test.h5';
buffer = load(inputfile);
expectedbuffer = struct('fs',1, 'offset',0, 'overlap',0, 'samples',(0:10:90)');

if ~isequal(buffer, expectedbuffer)
    error(['Error. ' inputfile ' does not match expected data']);
end

t_N=fieldnames(expectedbuffer);
for t_n=1:numel(t_N)
    eval([t_N{t_n} '= expectedbuffer.(t_N{t_n});']);
end

save('-hdf5', 'octaveoutput.h5', t_N{:});
