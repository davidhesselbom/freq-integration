% For use with GNU Octave 4.2.0
% Verify that all the RandomData and Results files are equal
% in each set on both machines, for each FFT library

function sanity()
%	compareRandomDataAcrossSets("Fusion")
%	compareRandomDataAcrossSets("Rampage")
%	compareFirstSetOfRandomDataAcrossMachines()
%	compareLibraryResultsAcrossSets("Fusion", "Ooura");
%	compareLibraryResultsAcrossSets("Fusion", "ClFft");
%	compareLibraryResultsAcrossSets("Fusion", "ClAmdFft");
%	compareLibraryResultsAcrossSets("Rampage", "Ooura");
%	compareLibraryResultsAcrossSets("Rampage", "ClFft");
%	compareLibraryResultsAcrossSets("Rampage", "ClAmdFft");
%	compareFirstSetOfLibraryResultsAcrossMachines("Ooura");
	compareFirstSetOfLibraryResultsAcrossMachines("ClFft");
%	compareFirstSetOfLibraryResultsAcrossMachines("ClAmdFft");

end

function compareChunkFiles(firstFileName, secondFileName)
	firstFile = load(firstFileName);
	secondFile = load(secondFileName);
	if (max(firstFile.chunk - secondFile.chunk) != 0);
		m = maxerr(firstFile.chunk(1:end), secondFile.chunk(1:end));
		n = nrmsd(firstFile.chunk(1:end), secondFile.chunk(1:end));
		disp(sprintf("%s and %s differ! MaxErr: %i, NRMSD: %i", firstFileName, secondFileName, m, n))
	else
		disp(sprintf("%s and %s are identical.", firstFileName, secondFileName))
	end
end

function compareSizeFiles(firstFileName, secondFileName)
	firstFile = load(firstFileName);
	secondFile = load(secondFileName);
	if (max(firstFile - secondFile) != 0);
		m = maxerr(firstFile(1:end), secondFile(1:end));
		n = nrmsd(firstFile(1:end), secondFile(1:end));
		disp(sprintf("%s and %s differ! MaxErr: %i, NRMSD: %i", firstFileName, secondFileName, m, n))
	else
		disp(sprintf("%s and %s are identical.", firstFileName, secondFileName))
	end
end


function compareRandomDataAcrossSets(machine)
	for set = 2:3
		for i = 1:5
			firstFileName = sprintf("C:/data/%s/set1/RandomData%i.h5", machine, i);
			secondFileName = sprintf("C:/data/%s/set%i/RandomData%i.h5", machine, set, i);
			compareChunkFiles(firstFileName, secondFileName);
		end
		disp("");
	end
end

function compareFirstSetOfRandomDataAcrossMachines()
	for i = 1:5
		fusionFile = sprintf("C:/data/Fusion/set1/RandomData%i.h5", i);
		rampageFile = sprintf("C:/data/Rampage/set1/RandomData%i.h5", i);
		compareChunkFiles(fusionFile, rampageFile);
	end
	disp("");
end

function resultsSanity(machine, techlib)
	for set = 1:3
		sizefile = sprintf("C:/data/Fusion/%s/Sizes.dat", techlib);
		disp(sprintf("Now comparing %s results...", techlib))
		if exist(sizefile)
			sizes = load(sizefile);
			for i = 1:5
				for size = sizes'
					fusionfile = sprintf("C:/data/Fusion/%s/run%i/Results%i.h5", techlib, i, size);
					rampagefile = sprintf("C:/data/Rampage/%s/run%i/Results%i.h5", techlib, i, size);

					compareChunkFiles(fusionfile, rampagefile);
				end
			end
		end
	end
end

function compareLibraryResultsAcrossSets(machine, techlib)
	firstSizeFile = sprintf("C:/data/%s/set1/%s/Sizes.dat", machine, techlib);
	for set = 2:3
		secondSizeFile = sprintf("C:/data/%s/set%i/%s/Sizes.dat", machine, set, techlib);
		compareSizeFiles(firstSizeFile, secondSizeFile)
	end
	
	sizes = load(firstSizeFile);

	for set = 2:3
		disp(sprintf("Now comparing %s results from %s, set %i...", techlib, machine, set))
		for i = 1:5
			for size = sizes'
				firstFile = sprintf("C:/data/%s/set1/%s/run%i/Results%i.h5", machine, techlib, i, size);
				secondFile = sprintf("C:/data/%s/set%i/%s/run%i/Results%i.h5", machine, set, techlib, i, size);

				compareChunkFiles(firstFile, secondFile);
			end
		end
	end
end

function compareFirstSetOfLibraryResultsAcrossMachines(techlib)
	firstSizeFile = sprintf("C:/data/Fusion/set1/%s/Sizes.dat", techlib);
	secondSizeFile = sprintf("C:/data/Rampage/set1/%s/Sizes.dat", techlib);
	compareSizeFiles(firstSizeFile, secondSizeFile)
	
	sizes = load(firstSizeFile);

	disp(sprintf("Now comparing %s results across machines...", techlib))

	for i = 1:5
		for size = sizes'
			firstFile = sprintf("C:/data/Fusion/set1/%s/run%i/Results%i.h5", techlib, i, size);
			secondFile = sprintf("C:/data/Rampage/set1/%s/run%i/Results%i.h5", techlib, i, size);

			compareChunkFiles(firstFile, secondFile);
		end
	end
end

function output = nrmsd(X, x)
	output = norm(X-x)/norm(X-mean(X));
end

function output = maxerr(X, x)
	output = max(abs(X-x));
end
