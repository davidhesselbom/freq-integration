% For use with GNU Octave 4.2.0
% Verify that all the RandomData and Results files are equal
% in each set on both machines, for each FFT library

function sanity()
	tic()
	compareOutput();
	computePrecision("Fusion", "Ooura");
	computePrecision("Fusion", "ClFft");
	computePrecision("Fusion", "ClAmdFft");
	computePrecision("Rampage", "ClFft");
	compareMaxErr("Fusion", "ClFft", "Fusion", "Ooura");
	compareMaxErr("Rampage", "ClFft", "Fusion", "Ooura");
	compareMaxErr("Fusion", "ClFft", "Rampage", "ClFft");
	compareBatchOutput();
	computeBatchPrecision();
	toc()
end

function compareOutput()
	% TODO: At some point, verify output from Octave fft is different from its input
	compareRandomDataAcrossMachines();
	compareFftOutputFromOctave();
	compareLibraryResultsAcrossMachines("Ooura");
	compareLibraryResultsAcrossMachines("ClFft");
	compareLibraryResultsAcrossMachines("ClAmdFft");
end

function compareBatchOutput()
	compareBatchRandomDataToRandomData()
	compareBatchRandomDataAcrossMachines()
	compareBatchFftOutputFromOctave();
	compareBatchResultsAcrossMachines();
	compareOutputOfBatchSize1toBenchResults();
	compareTimesOfBatchSize1toBench("Fusion");
	compareTimesOfBatchSize1toBench("Rampage");
end

function compareChunkFiles(firstFileName, secondFileName)
	firstFile = load(firstFileName);
	secondFile = load(secondFileName);
	if (max(firstFile.chunk - secondFile.chunk) != 0);
		m = maxerr(firstFile.chunk, secondFile.chunk);
		n = nrmsd(firstFile.chunk, secondFile.chunk);
		disp(sprintf("%s and %s differ! MaxErr: %i, NRMSD: %i", firstFileName, secondFileName, m, n))
	else
		%disp(sprintf("%s and %s are identical.", firstFileName, secondFileName))
	end
end

function compareSizeFiles(firstFileName, secondFileName)
	disp("Comparing size files...");
	firstFile = load(firstFileName);
	secondFile = load(secondFileName);
	if (max(firstFile - secondFile) != 0);
		m = maxerr(firstFile, secondFile);
		n = nrmsd(firstFile, secondFile);
		disp(sprintf("%s and %s differ! MaxErr: %i, NRMSD: %i", firstFileName, secondFileName, m, n))
	else
		%disp(sprintf("%s and %s are identical.", firstFileName, secondFileName))
	end
end

function compareFftOutputFromOctave()
	sizes = load(sprintf("C:/data/Fusion/Ooura/Sizes.dat"));

	for set = 1:5
		disp(sprintf("Computing precision for Octave vs Octave, set %i...", set));
		randomData = load(sprintf("C:/data/Fusion/RandomData%i.h5", set));
		for size = sizes'
			reference = fft(randomData.chunk(1:size));
			reference2 = fft(randomData.chunk(1:size));
			if (max(reference - reference2) != 0);
				m = maxerr(reference, reference2);
				n = nrmsd(reference, reference2);
				disp(sprintf("Size: %i: Octave results differ! MaxErr: %i, NRMSD: %i", size, m, n))
			else
				%disp(sprintf("Octave results are identical."))
			end
		end
	end
	disp("");
end

function compareRandomDataAcrossMachines()
	disp("Comparing random data files from Fusion to Rampage...");
	for set = 1:5
		fusionFile = sprintf("C:/data/Fusion/RandomData%i.h5", set);
		rampageFile = sprintf("C:/data/Rampage/RandomData%i.h5", set);
		compareChunkFiles(fusionFile, rampageFile);
	end
	disp("");
end

function compareLibraryResultsAcrossMachines(techlib)
	firstSizeFile = sprintf("C:/data/Fusion/%s/Sizes.dat", techlib);
	secondSizeFile = sprintf("C:/data/Rampage/%s/Sizes.dat", techlib);
	compareSizeFiles(firstSizeFile, secondSizeFile)
	
	sizes = load(firstSizeFile);

	disp(sprintf("Comparing %s results across machines...", techlib))

	for set = 1:5
		for size = sizes'
			firstFile = sprintf("C:/data/Fusion/%s/set%i/Results%i.h5", techlib, set, size);
			secondFile = sprintf("C:/data/Rampage/%s/set%i/Results%i.h5", techlib, set, size);

			compareChunkFiles(firstFile, secondFile);
		end
	end
	disp("");
end

function compareBatchRandomDataToRandomData()
	disp(sprintf("Comparing batch random data files to random data files from Fusion..."));
	for set = 1:5
		firstFile = load(sprintf("C:/data/Fusion/BatchRandomData%i.h5", set)).chunk(1:2^22);
		secondFile = load(sprintf("C:/data/Fusion/RandomData%i.h5", set)).chunk;
		if (max(firstFile - secondFile) != 0);
			m = maxerr(firstFile, secondFile);
			n = nrmsd(firstFile, secondFile);
			disp(sprintf("Set: %i: Random data differs! MaxErr: %i, NRMSD: %i", set, m, n))
		else
			%disp(sprintf("Random data is identical."))
		end
	end
	disp("");
end

function compareBatchRandomDataAcrossMachines()
	disp("Comparing batch random data files from Fusion to Rampage...");
	for set = 1:5
		fusionFile = sprintf("C:/data/Fusion/BatchRandomData%i.h5", set);
		rampageFile = sprintf("C:/data/Rampage/BatchRandomData%i.h5", set);
		compareChunkFiles(fusionFile, rampageFile);
	end
	disp("");
end

function compareOutputOfBatchSize1toBenchResults()
	disp("Comparing batch output to no-batch output...");
	for set = 1:5
		firstFile = load(sprintf("C:/data/Fusion/ClAmdFft/set%i/16384Results1024.h5", set)).chunk(1:1024);
		secondFile = load(sprintf("C:/data/Fusion/ClAmdFft/set%i/Results1024.h5", set)).chunk;
		if (max(firstFile - secondFile) != 0);
			m = maxerr(firstFile, secondFile);
			n = nrmsd(firstFile, secondFile);
			disp(sprintf("Set: %i: Batch size 1 output differs from bench output! MaxErr: %i, NRMSD: %i", set, m, n))
		else
			%disp(sprintf("Random data is identical."))
		end
	end
	disp("");
end

function compareTimesOfBatchSize1toBench(machine)
	wallTimes = [];
	kernelExecutionTimes = [];
	batchWallTimes = [];
	batchKernelExecutionTimes = [];
	for set = 1:5
		for run = 1:3
			wallTimes = [wallTimes, load(sprintf("C:/data/%s/ClAmdFft/set%i/run%i/WallTimes.dat", machine, set, run))(13, 3:end)];
			batchWallTimes = [batchWallTimes, load(sprintf("C:/data/%s/ClAmdFft/set%i/batch%i/WallTimes1024.dat", machine, set, run))(end, 3:end)];

			kernelExecutionTimes = [kernelExecutionTimes, load(sprintf("C:/data/%s/ClAmdFft/set%i/run%i/KernelExecutionTimes.dat", machine, set, run))(13, 3:end)];
			batchKernelExecutionTimes = [batchKernelExecutionTimes, load(sprintf("C:/data/%s/ClAmdFft/set%i/batch%i/KernelExecutionTimes1024.dat", machine, set, run))(end, 3:end)];
		end
	end

	meanWallTimesDiff = 100*abs((mean(wallTimes) - mean(batchWallTimes))/mean(wallTimes));
	meanKernelExecutionTimesDiff = 100*abs((mean(kernelExecutionTimes) - mean(batchKernelExecutionTimes))/mean(kernelExecutionTimes));
	disp(sprintf("Mean walltime difference between no batch and single batch for 1024 on %s: %i %%", machine, meanWallTimesDiff));
	disp(sprintf("Mean kernel execution time difference between no batch and single batch for 1024 on %s: %i %%", machine, meanKernelExecutionTimesDiff));
	% TODO: Find a more suitable way to present the below numbers. Box plot?
	disp(min(wallTimes));
	disp(max(wallTimes));
	disp(min(batchWallTimes));
	disp(max(batchWallTimes));
	disp(min(kernelExecutionTimes));
	disp(max(kernelExecutionTimes));
	disp(min(batchKernelExecutionTimes));
	disp(max(batchKernelExecutionTimes));

	disp("");
end

function compareBatchFftOutputFromOctave()
	for set = 1:5
		disp(sprintf("Computing precision for batches vs Octave, set %i...", set));
		randomData = load(sprintf("C:/data/Fusion/BatchRandomData%i.h5", set));
		for slice = 1:2^14;
			reference = fft(randomData.chunk(1+(slice-1)*1024:slice*1024));
			reference2 = fft(randomData.chunk(1+(slice-1)*1024:slice*1024));
			if (max(reference - reference2) != 0);
				m = maxerr(reference, reference2);
				n = nrmsd(reference, reference2);
				disp(sprintf("Size: %i: Octave results differ! MaxErr: %i, NRMSD: %i", size, m, n))
			else
				%disp(sprintf("Octave results are identical."))
			end
		end
	end
	disp("");
end

function compareBatchResultsAcrossMachines()
	disp(sprintf("Comparing batch results from Fusion to Rampage..."));
	for set = 1:5
		for batchSize = 2.^(14)
			firstFile = sprintf("C:/data/Fusion/ClAmdFft/set%i/%iResults1024.h5", set, batchSize);
			secondFile = sprintf("C:/data/Rampage/ClAmdFft/set%i/%iResults1024.h5", set, batchSize);

			compareChunkFiles(firstFile, secondFile);
		end
	end
	disp("");
end

function computeBatchPrecision()
	% For each set from one of the machines, slice largest batch result into pieces of 1024,
	% compare each piece with fft result from Octave, and store the maximum maxerr and NRMSD.
	% See computePrecision below, use a "vectorToSave"

	% TODO: Use this instead of computePrecision for all libraries, and all sizes.
	for set = 1:5
		disp(sprintf("Computing batch precision, set %i...", set));
		randomData = load(sprintf("C:/data/Fusion/BatchRandomData%i.h5", set));
		resultsFile = load(sprintf("C:/data/Fusion/ClAmdFft/set%i/%iResults1024.h5", set, 2^14));

		for slice = 1:2^14
			randomSlice = randomData.chunk(1+(slice-1)*1024:slice*1024);
			resultsSlice = resultsFile.chunk(1+(slice-1)*1024:slice*1024);

			reference = fft(randomSlice);
			startElement = 2;
			m((set-1)*(2^14)+slice) = maxerr(resultsSlice(startElement:end), reference(startElement:end));
			n((set-1)*(2^14)+slice) = nrmsd(resultsSlice(startElement:end), reference(startElement:end));
			if (set == 5)
				vectorToSave(1) = max(m)/sqrt(1024);
				vectorToSave(2) = max(n);
			end
		end
	end
	disp(sprintf("Maximum Maxerr: %i, Maximum NRMSD: %i", max(m), max(n)))
	save("BatchPrecision.dat", "vectorToSave");
end

function computePrecision(machine, techlib)
	sizes = load(sprintf("C:/data/%s/%s/Sizes.dat", machine, techlib));
	
	for set = 1:5
		disp(sprintf("Computing precision for %s on %s, set %i...", techlib, machine, set));
		randomData = load(sprintf("C:/data/%s/RandomData%i.h5", machine, set));

		for index = 1:size(sizes', 2)
			currentSize = sizes(index);
			resultsFile = load(sprintf("C:/data/%s/%s/set%i/Results%i.h5", machine, techlib, set, currentSize));
			reference = fft(randomData.chunk(1:currentSize));
			% TODO: Does element 1 really need to be excluded?
			startElement = 2;
			m(set,index) = maxerr(resultsFile.chunk(startElement:end), reference(startElement:end));
			n(set,index) = nrmsd(resultsFile.chunk(startElement:end), reference(startElement:end));
			%disp(sprintf("Size: %i, MaxErr: %i, NRMSD: %i", currentSize, m, n))
			if (set == 5)
				vectorToSave(index,1) = currentSize;
				vectorToSave(index,2) = max(m(:,index))/sqrt(currentSize);
				vectorToSave(index,3) = max(n(:,index));
				disp(sprintf("Size: %i, Maximum Maxerr: %i, Maximum NRMSD: %i", vectorToSave(index,1), vectorToSave(index,2), vectorToSave(index,3)))
			end
		end
	end
	save(sprintf("%sPrecision%s.dat", techlib, machine), "vectorToSave");
	disp("");
end

function compareMaxErr(machine1, techlib1, machine2, techlib2)
	sizes = load(sprintf("C:/data/%s/%s/Sizes.dat", machine1, techlib1));

	for set = 1:5
		disp(sprintf("Comparing Maxerr for %s on %s with %s on %s, set %i...", techlib1, machine1, techlib2, machine2, set));
		randomData = load(sprintf("C:/data/%s/RandomData%i.h5", machine1, set));

		for index = 1:size(sizes', 2)
			currentSize = sizes(index);
			resultsFile1 = load(sprintf("C:/data/%s/%s/set%i/Results%i.h5", machine1, techlib1, set, currentSize));
			resultsFile2 = load(sprintf("C:/data/%s/%s/set%i/Results%i.h5", machine2, techlib2, set, currentSize));
			reference = fft(randomData.chunk(1:currentSize));
			% TODO: Does element 1 really need to be excluded?
			startElement = 2;
			m1(set,index) = maxerr(resultsFile1.chunk(startElement:end), reference(startElement:end));
			m2(set,index) = maxerr(resultsFile2.chunk(startElement:end), reference(startElement:end));
			%disp(sprintf("Size: %i, MaxErr: %i, NRMSD: %i", currentSize, m, n))
			if (set == 5)
				if (m1(set,index) != m2(set,index))
					disp(sprintf("Size: %i, Maxerr diff: %i", currentSize, abs(m1(set,index) - m2(set,index))))
				end
			end
		end
	end
	disp("");
end

function output = nrmsd(X, x)
	output = norm(X-x)/norm(X-mean(X));
end

function output = maxerr(X, x)
	output = max(abs(X-x));
end
