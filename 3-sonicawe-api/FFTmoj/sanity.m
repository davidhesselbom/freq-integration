% For use with GNU Octave 4.2.0
% Verify that all the RandomData and Results files are equal
% in each set on both machines, for each FFT library

function sanity(dataPath)
	tic()
	compareOutput(dataPath);
	compareMaxErr(dataPath, "Fusion", "ClFft", "Fusion", "Ooura");
	compareMaxErr(dataPath, "Rampage", "ClFft", "Fusion", "Ooura");
	compareMaxErr(dataPath, "Fusion", "ClFft", "Rampage", "ClFft");
	computeBatchPrecision(dataPath);
	computePrecisionGroup(dataPath);
	compareBatchFftOutputFromOctave(dataPath);
	toc()
end

function compareOutput(dataPath)
	compareRandomDataAcrossMachines(dataPath);
	compareFftOutputFromOctave(dataPath);
	compareLibraryResultsAcrossMachines(dataPath, "Ooura");
	compareLibraryResultsAcrossMachines(dataPath, "ClFft");
	compareLibraryResultsAcrossMachines(dataPath, "ClAmdFft");
end

function computePrecisionGroup(dataPath)
	computePrecision(dataPath, "Fusion", "Ooura");
	computePrecision(dataPath, "Fusion", "ClAmdFft");
	computePrecision(dataPath, "Fusion", "ClFft");
	computePrecision(dataPath, "Rampage", "ClFft");
end

function compareChunkFiles(firstFileName, secondFileName)
	firstFile = load(firstFileName);
	secondFile = load(secondFileName);
	if (firstFile.chunk != secondFile.chunk);
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
	if (firstFile != secondFile);
		m = maxerr(firstFile, secondFile);
		n = nrmsd(firstFile, secondFile);
		disp(sprintf("%s and %s differ! MaxErr: %i, NRMSD: %i", firstFileName, secondFileName, m, n))
	else
		%disp(sprintf("%s and %s are identical.", firstFileName, secondFileName))
	end
end

function compareFftOutputFromOctave(dataPath)
	sizes = load(sprintf("%s/Fusion/Ooura/Sizes.dat", dataPath));

	for set = 1:5
		disp(sprintf("Computing precision for Octave vs Octave, set %i...", set));
		randomData = load(sprintf("%s/Fusion/RandomData%i.h5", dataPath, set));
		for size = sizes'
			reference = fft(randomData.chunk(1:size));
			reference2 = fft(randomData.chunk(1:size));
			if (randomData.chunk(1:size) == reference)
				disp("PANIC: Octave's fft results are identical to input!");
			end
			if (reference != reference2);
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

function compareRandomDataAcrossMachines(dataPath)
	disp("Comparing random data files from Fusion to Rampage...");
	for set = 1:5
		fusionFile = sprintf("%s/Fusion/RandomData%i.h5", dataPath, set);
		rampageFile = sprintf("%s/Rampage/RandomData%i.h5", dataPath, set);
		compareChunkFiles(fusionFile, rampageFile);
	end
	disp("");
end

function compareLibraryResultsAcrossMachines(dataPath, techlib)
	firstSizeFile = sprintf("%s/Fusion/%s/Sizes.dat", dataPath, techlib);
	secondSizeFile = sprintf("%s/Rampage/%s/Sizes.dat", dataPath, techlib);
	compareSizeFiles(firstSizeFile, secondSizeFile)
	
	sizes = load(firstSizeFile);

	disp(sprintf("Comparing %s results across machines...", techlib))

	for set = 1:5
		for size = sizes'
			firstFile = sprintf("%s/Fusion/%s/set%i/Results%i.h5", dataPath, techlib, set, size);
			secondFile = sprintf("%s/Rampage/%s/set%i/Results%i.h5", dataPath, techlib, set, size);

			compareChunkFiles(firstFile, secondFile);
		end
	end
	disp("");
end

function compareBatchFftOutputFromOctave(dataPath)
	% Verify Octave returns the same result if slicing input the same way FFTmoj does for batches
	% TODO: This needs doing for all sizes of FFT, not just 1024
	for set = 1:5
		disp(sprintf("Computing precision for batches vs Octave, set %i...", set));
		randomData = load(sprintf("%s/Fusion/RandomData%i.h5", dataPath, set));
		for slice = 1:2^14;
			reference = fft(randomData.chunk(1+(slice-1)*1024:slice*1024));
			reference2 = fft(randomData.chunk(1+(slice-1)*1024:slice*1024));
			if (reference != reference2);
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

function computePrecision(dataPath, machine, techlib)
	% For each size in each set from one techlib from one of the machines, slice result file,
	% compare each slice with fft result from Octave, and store the maximum maxerr and NRMSD.
	sizes = load(sprintf("%s/%s/%s/Sizes.dat", dataPath, machine, techlib));

	vectorToSave = zeros(rows(sizes), 5);
	m = zeros(5, rows(sizes));
	n = zeros(5, rows(sizes));

	for set = 1:5
		printf("Computing precision for %s on %s, set %i ", techlib, machine, set);
		randomData = load(sprintf("%s/%s/RandomData%i.h5", dataPath, machine, set));

		for index = 1:rows(sizes)
			currentSize = sizes(index);
			resultsFile = load(sprintf("%s/%s/%s/set%i/Results%i.h5", dataPath, machine, techlib, set, currentSize));
			for slice = 1:rows(resultsFile.chunk)/currentSize
				randomSlice = randomData.chunk(1+(slice-1)*currentSize:slice*currentSize);
				resultsSlice = resultsFile.chunk(1+(slice-1)*currentSize:slice*currentSize);
				reference = fft(randomSlice);
				% TODO: Does element 1 really need to be excluded?
				startElement = 2;
				m(set,index) = max(m(set,index), maxerr(resultsSlice(startElement:end), reference(startElement:end)));
				n(set,index) = max(n(set,index), nrmsd(resultsSlice(startElement:end), reference(startElement:end)));
				%disp(sprintf("Size: %i, MaxErr: %i, NRMSD: %i", currentSize, m, n))
			end
			if (set == 5)
				if (index == 1)
					printf("\n");
				end
				vectorToSave(index,1) = currentSize;
				vectorToSave(index,2) = max(m(:,index));
				vectorToSave(index,3) = max(n(:,index));
				vectorToSave(index,4) = vectorToSave(index,2)/sqrt(currentSize);
				vectorToSave(index,5) = vectorToSave(index,3)/sqrt(currentSize);
				disp(sprintf("Size: %i, Maximum Maxerr: %i, Maximum NRMSD: %i", vectorToSave(index,1), vectorToSave(index,2), vectorToSave(index,3)))
			else
				printf(".");
			end
		end
		printf("\n");
	end
	save(sprintf("%sPrecision%s.dat", techlib, machine), "vectorToSave");
	disp("");
end

function compareMaxErr(dataPath, machine1, techlib1, machine2, techlib2)
	sizes = load(sprintf("%s/%s/%s/Sizes.dat", dataPath, machine1, techlib1));

	for set = 1:5
		disp(sprintf("Comparing Maxerr for %s on %s with %s on %s, set %i...", techlib1, machine1, techlib2, machine2, set));
		randomData = load(sprintf("%s/%s/RandomData%i.h5", dataPath, machine1, set));

		for index = 1:size(sizes', 2)
			currentSize = sizes(index);
			resultsFile1 = load(sprintf("%s/%s/%s/set%i/Results%i.h5", dataPath, machine1, techlib1, set, currentSize));
			resultsFile2 = load(sprintf("%s/%s/%s/set%i/Results%i.h5", dataPath, machine2, techlib2, set, currentSize));
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
