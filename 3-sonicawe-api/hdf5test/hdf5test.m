if ~exist("OCTAVE_VERSION","builtin")
    error('Supposed to be run from Octave');
end

inputfile = 'hdf5test.h5';
buffer = load(inputfile);
expectedbuffer = struct('fs',1, 'offset',0, 'overlap',0, 'samples',[(0:10:90)' (1000:10:1090)']);

if ~isequal(buffer, expectedbuffer)
    error(['Error. ' inputfile ' does not match expected data']);
end

t_N=fieldnames(expectedbuffer);
for t_n=1:numel(t_N)
    eval([t_N{t_n} '= expectedbuffer.(t_N{t_n});']);
end

save('-hdf5', 'octaveoutput.h5', t_N{:});

inputfile2 = 'hdf5testchunk.h5';
chunkstruct = load(inputfile2);

if ~isfield(chunkstruct,'chunk')
    error(['Error. ' inputfile2 ' does not match expected data']);
end

samples = expectedbuffer.samples(1:8,1);
d=fft(samples) - chunkstruct.chunk;
if max(max(abs(d))) > 1e-7
    error(['Error. ' inputfile2 ' does not match expected data']);
end

if ~isequal(samples, round(ifft(chunkstruct.chunk)))
    error(['Error. ' inputfile2 ' does not match expected data']);
end

% write back the same structure that was read
t_N=fieldnames(chunkstruct);
for t_n=1:numel(t_N)
    eval([t_N{t_n} '= chunkstruct.(t_N{t_n});']);
end

save('-hdf5', 'octavechunk.h5', t_N{:});
