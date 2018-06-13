% For use with GNU Octave 4.2.0
% Verify that all the RandomData and Results files are equal
% in each set on both machines, for each FFT library

function sanity()
	tic()
	%compareOutput();
	%computePrecision("Fusion", "Ooura");
	%computePrecision("Fusion", "ClFft");
	%computePrecision("Fusion", "ClAmdFft");
	%computePrecision("Rampage", "ClFft");
	%compareMaxErr("Fusion", "Ooura", "Fusion", "ClFft");
	%compareMaxErr("Fusion", "Ooura", "Rampage", "ClFft");
	%compareMaxErr("Fusion", "ClFft", "Rampage", "ClFft");
	%compareBatchOutput();
	computeBatchPrecision();
	toc()
end

function compareOutput()
	% TODO: At some point, verify output from Octave fft is different from its input
	% TODO: At some point, verify outout from FFTmoj is different from its input
	compareRandomDataAcrossSets("Fusion")
	compareRandomDataAcrossSets("Rampage")
	compareFirstSetOfRandomDataAcrossMachines();
	%compareFftOutputFromOctave();
	compareLibraryResultsAcrossSets("Fusion", "Ooura");
	compareLibraryResultsAcrossSets("Fusion", "ClFft");
	compareLibraryResultsAcrossSets("Fusion", "ClAmdFft");
	compareLibraryResultsAcrossSets("Rampage", "Ooura");
	compareLibraryResultsAcrossSets("Rampage", "ClFft");
	compareLibraryResultsAcrossSets("Rampage", "ClAmdFft");
	compareFirstSetOfLibraryResultsAcrossMachines("Ooura");
	compareFirstSetOfLibraryResultsAcrossMachines("ClFft");
	compareFirstSetOfLibraryResultsAcrossMachines("ClAmdFft");
end

function compareBatchOutput()
	%compareBatchRandomDataAcrossSets("Fusion")
	%compareBatchRandomDataAcrossSets("Rampage")
	%compareBatchRandomDataToRandomData()
	%compareFirstSetOfBatchRandomDataAcrossMachines()
	%compareBatchFftOutputFromOctave();
	%compareBatchResultsAcrossSets("Fusion");
	%compareBatchResultsAcrossSets("Rampage");
	%compareFirstSetOfBatchResultsAcrossMachines();
	%compareOutputOfSmallerBatchesToLargestBatch();
	%compareOutputOfBatchSize1toBenchResults();
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
	sizes = load(sprintf("C:/data/Fusion/set1/Ooura/Sizes.dat"));

	for run = 1:5
		disp(sprintf("Computing precision for Octave vs Octave, run %i...", run));
		randomData = load(sprintf("C:/data/Fusion/set1/RandomData%i.h5", run));
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

function compareRandomDataAcrossSets(machine)
	for set = 2:3
			disp(sprintf("Comparing random data files from %s, set 1 to set %i...", machine, set));
		for i = 1:5
			firstFileName = sprintf("C:/data/%s/set1/RandomData%i.h5", machine, i);
			secondFileName = sprintf("C:/data/%s/set%i/RandomData%i.h5", machine, set, i);
			compareChunkFiles(firstFileName, secondFileName);
		end
	end
	disp("");
end

function compareFirstSetOfRandomDataAcrossMachines()
	disp("Comparing random data files from Fusion to Rampage, set 1...");
	for i = 1:5
		fusionFile = sprintf("C:/data/Fusion/set1/RandomData%i.h5", i);
		rampageFile = sprintf("C:/data/Rampage/set1/RandomData%i.h5", i);
		compareChunkFiles(fusionFile, rampageFile);
	end
	disp("");
end

function compareLibraryResultsAcrossSets(machine, techlib)
	disp(sprintf("Comparing %s sizes files from %s...", techlib, machine))
	firstSizeFile = sprintf("C:/data/%s/set1/%s/Sizes.dat", machine, techlib);
	for set = 2:3
		secondSizeFile = sprintf("C:/data/%s/set%i/%s/Sizes.dat", machine, set, techlib);
		compareSizeFiles(firstSizeFile, secondSizeFile)
	end
	
	sizes = load(firstSizeFile);

	for set = 2:3
		disp(sprintf("Comparing %s results from %s, set 1 to set %i...", techlib, machine, set))
		for i = 1:5
			for size = sizes'
				firstFile = sprintf("C:/data/%s/set1/%s/run%i/Results%i.h5", machine, techlib, i, size);
				secondFile = sprintf("C:/data/%s/set%i/%s/run%i/Results%i.h5", machine, set, techlib, i, size);

				compareChunkFiles(firstFile, secondFile);
			end
		end
	end
	disp("");
end

function compareFirstSetOfLibraryResultsAcrossMachines(techlib)
	firstSizeFile = sprintf("C:/data/Fusion/set1/%s/Sizes.dat", techlib);
	secondSizeFile = sprintf("C:/data/Rampage/set1/%s/Sizes.dat", techlib);
	compareSizeFiles(firstSizeFile, secondSizeFile)
	
	sizes = load(firstSizeFile);

	disp(sprintf("Comparing %s results across machines...", techlib))

	for i = 1:5
		for size = sizes'
			firstFile = sprintf("C:/data/Fusion/set1/%s/run%i/Results%i.h5", techlib, i, size);
			secondFile = sprintf("C:/data/Rampage/set1/%s/run%i/Results%i.h5", techlib, i, size);

			compareChunkFiles(firstFile, secondFile);
		end
	end
	disp("");
end

function compareBatchRandomDataAcrossSets(machine)
	for set = 2:3
			disp(sprintf("Comparing batch random data files from %s, set 1 to set %i...", machine, set));
		for i = 1:5
			firstFileName = sprintf("C:/data/%s/set1/BatchRandomData%i.h5", machine, i);
			secondFileName = sprintf("C:/data/%s/set%i/BatchRandomData%i.h5", machine, set, i);
			compareChunkFiles(firstFileName, secondFileName);
		end
	end
	disp("");
end

function compareBatchRandomDataToRandomData()
	disp(sprintf("Comparing batch random data files to random data files from Fusion, set 1 to set 1..."));
	for i = 1:5
		firstFile = load(sprintf("C:/data/Fusion/set1/BatchRandomData%i.h5", i)).chunk(1:2^22);
		secondFile = load(sprintf("C:/data/Fusion/set1/RandomData%i.h5", i)).chunk;
		if (max(firstFile - secondFile) != 0);
			m = maxerr(firstFile, secondFile);
			n = nrmsd(firstFile, secondFile);
			disp(sprintf("Run: %i: Random data differs! MaxErr: %i, NRMSD: %i", i, m, n))
		else
			%disp(sprintf("Random data is identical."))
		end
	end
	disp("");
end

function compareFirstSetOfBatchRandomDataAcrossMachines()
	disp("Comparing batch random data files from Fusion to Rampage, set 1...");
	for i = 1:5
		fusionFile = sprintf("C:/data/Fusion/set1/BatchRandomData%i.h5", i);
		rampageFile = sprintf("C:/data/Rampage/set1/BatchRandomData%i.h5", i);
		compareChunkFiles(fusionFile, rampageFile);
	end
	disp("");
end

function compareOutputOfSmallerBatchesToLargestBatch()
	for run = 1:5
		disp(sprintf("Comparing batch results from Fusion, smaller batches to largest batch, run %i...", run));
		firstFile = load(sprintf("C:/data/Fusion/set1/ClAmdFft/batch%i/%iResults1024.h5", run, 2^14)).chunk;
		for batchSize = 2.^(0:13)
			secondFile = load(sprintf("C:/data/Fusion/set1/ClAmdFft/batch%i/%iResults1024.h5", run, batchSize)).chunk;
			if (max(firstFile(1:1024*batchSize) - secondFile) != 0);
				m = maxerr(firstFile(1:1024*batchSize), secondFile);
				n = nrmsd(firstFile(1:1024*batchSize), secondFile);
				disp(sprintf("Run: %i, Batch size: %i: Batch result differs from largets batch! MaxErr: %i, NRMSD: %i", i, batchSize, m, n))
			else
				%disp(sprintf("Batch results are identical."))
			end
		end
	end
	disp("");
end

function compareOutputOfBatchSize1toBenchResults()
	for run = 1:5
		firstFile = sprintf("C:/data/Fusion/set1/ClAmdFft/batch%i/1Results1024.h5", run);
		secondFile = sprintf("C:/data/Fusion/set1/ClAmdFft/run%i/Results1024.h5", run);
		compareChunkFiles(firstFile, secondFile);
	end
	disp("");
end

function compareTimesOfBatchSize1toBench(machine)
	wallTimes = [];
	kernelExecutionTimes = [];
	batchWallTimes = [];
	batchKernelExecutionTimes = [];
	for set = 1:3
		for run = 1:5
			wallTimes = [wallTimes, load(sprintf("C:/data/%s/set%i/ClAmdFft/run%i/WallTimes.dat", machine, set, run))(13, 3:end)];
			batchWallTimes = [batchWallTimes, load(sprintf("C:/data/%s/set%i/ClAmdFft/batch%i/WallTimes1024.dat", machine, set, run))(1, 3:end)];

			kernelExecutionTimes = [kernelExecutionTimes, load(sprintf("C:/data/%s/set%i/ClAmdFft/run%i/KernelExecutionTimes.dat", machine, set, run))(13, 3:end)];
			batchKernelExecutionTimes = [batchKernelExecutionTimes, load(sprintf("C:/data/%s/set%i/ClAmdFft/batch%i/KernelExecutionTimes1024.dat", machine, set, run))(1, 3:end)];
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
	for run = 1:5
		disp(sprintf("Computing precision for Octave vs Octave, run %i...", run));
		randomData = load(sprintf("C:/data/Fusion/set1/BatchRandomData%i.h5", run));
		for i = 1:2^14;
			reference = fft(randomData.chunk(1+(i-1)*1024:i*1024));
			reference2 = fft(randomData.chunk(1+(i-1)*1024:i*1024));
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

function compareBatchResultsAcrossSets(machine)
	for set = 2:3
		disp(sprintf("Comparing batch results from %s, set 1 to set %i...", machine, set));
		for run = 1:5
			for batchSize = 2.^(0:14)
				firstFile = sprintf("C:/data/%s/set1/ClAmdFft/batch%i/%iResults1024.h5", machine, run, batchSize);
				secondFile = sprintf("C:/data/%s/set%i/ClAmdFft/batch%i/%iResults1024.h5", machine, set, run, batchSize);

				compareChunkFiles(firstFile, secondFile);
			end
		end
		disp("");
	end
end

function compareFirstSetOfBatchResultsAcrossMachines()
	disp(sprintf("Comparing batch results from Fusion to Rampage, set 1..."));
	for run = 1:5
		for batchSize = 2.^(0:14)
			firstFile = sprintf("C:/data/Fusion/set1/ClAmdFft/batch%i/%iResults1024.h5", run, batchSize);
			secondFile = sprintf("C:/data/Rampage/set1/ClAmdFft/batch%i/%iResults1024.h5", run, batchSize);

			compareChunkFiles(firstFile, secondFile);
		end
	end
	disp("");
end

function computeBatchPrecision()
	% TODO: For each run in the first set from one machine, slice largest batch result into pieces of 1024, 
	% compare each piece with fft result from Octave, and store the maximum maxerr and NRMSD. 
	% See computePrecision below, use a "vectorToSave"

	for run = 1:5
		disp(sprintf("Computing batch precision, run %i...", run));
		randomData = load(sprintf("C:/data/Fusion/set1/BatchRandomData%i.h5", run));
		resultsFile = load(sprintf("C:/data/Fusion/set1/ClAmdFft/batch%i/%iResults1024.h5", run, 2^14));

		for i = 1:2^14
			randomSlice = randomData.chunk(1+(i-1)*1024:i*1024);
			resultsSlice = resultsFile.chunk(1+(i-1)*1024:i*1024);
			if (max(resultsSlice-randomSlice) == 0)
				% TODO: Make better error message
				disp(sprintf("PANIC!!! results equal input at batch slice %i", i))
				break
			end
			reference = fft(randomSlice);
			startElement = 2;
			m((run-1)*(2^14)+i) = maxerr(resultsSlice(startElement:end), reference(startElement:end));
			n((run-1)*(2^14)+i) = nrmsd(resultsSlice(startElement:end), reference(startElement:end));
			if (run == 5)
				vectorToSave(1) = max(m)/sqrt(1024);
				vectorToSave(2) = max(n);
			end
		end
	end
	disp(sprintf("Maximum Maxerr: %i, Maximum NRMSD: %i", max(m), max(n)))
	save("BatchPrecision.dat", "vectorToSave");
end

function computePrecision(machine, techlib)
	sizes = load(sprintf("C:/data/%s/set1/%s/Sizes.dat", machine, techlib));
	
	for run = 1:5
		disp(sprintf("Computing precision for %s on %s, run %i...", techlib, machine, run));
		randomData = load(sprintf("C:/data/%s/set1/RandomData%i.h5", machine, run));

		for i = 1:size(sizes', 2)
			currentSize = sizes(i);
			resultsFile = load(sprintf("C:/data/%s/set1/%s/run%i/Results%i.h5", machine, techlib, run, currentSize));
			reference = fft(randomData.chunk(1:currentSize));
			% TODO: Does element 1 really need to be excluded?
			startElement = 2;
			m(run,i) = maxerr(resultsFile.chunk(startElement:end), reference(startElement:end));
			n(run,i) = nrmsd(resultsFile.chunk(startElement:end), reference(startElement:end));
			%disp(sprintf("Size: %i, MaxErr: %i, NRMSD: %i", currentSize, m, n))
			if (run == 5)
				vectorToSave(i,1) = currentSize;
				vectorToSave(i,2) = max(m(:,i))/sqrt(currentSize);
				vectorToSave(i,3) = max(n(:,i));
				disp(sprintf("Size: %i, Maximum Maxerr: %i, Maximum NRMSD: %i", currentSize, max(m(:,i)), max(n(:,i))))
			end
		end
	end
	save(sprintf("%sPrecision%s.dat", techlib, machine), "vectorToSave");
	disp("");
end

function compareMaxErr(machine1, techlib1, machine2, techlib2)
	sizes = load(sprintf("C:/data/%s/set1/%s/Sizes.dat", machine1, techlib1));

	for run = 1:5
		disp(sprintf("Comparing Maxerr for %s on %s with %s on %s, run %i...", techlib1, machine1, techlib2, machine2, run));
		randomData = load(sprintf("C:/data/%s/set1/RandomData%i.h5", machine1, run));

		for i = 1:size(sizes', 2)
			currentSize = sizes(i);
			resultsFile1 = load(sprintf("C:/data/%s/set1/%s/run%i/Results%i.h5", machine1, techlib1, run, currentSize));
			resultsFile2 = load(sprintf("C:/data/%s/set1/%s/run%i/Results%i.h5", machine2, techlib2, run, currentSize));
			reference = fft(randomData.chunk(1:currentSize));
			% TODO: Does element 1 really need to be excluded?
			startElement = 2;
			m1(run,i) = maxerr(resultsFile1.chunk(startElement:end), reference(startElement:end));
			m2(run,i) = maxerr(resultsFile2.chunk(startElement:end), reference(startElement:end));
			%disp(sprintf("Size: %i, MaxErr: %i, NRMSD: %i", currentSize, m, n))
			if (run == 5)
				if (m1(run,i) != m2(run,i))
					disp(sprintf("Size: %i, Maxerr diff: %i", currentSize, abs(m1(run,i) - m2(run,i))))
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
