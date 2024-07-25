classdef Plotter < GenericProcessor
	% Plotter An example of a sink processor that displays the incoming data in a plot.
	%	This processor will continuously plot the incoming data until the figure is closed. 
	%
	% Plotter Properties
	%	hPlot - The handle to the plot

	properties
		hPlot;
        fig;
    end
    
    properties
        xAxisRange;
        yAxisRange;
    end

	properties (Access = private)
		numChannels;
		sampleRate;
	end

    methods 

    	function self = Plotter(host, port)
            
            self = self@GenericProcessor(host, port);
            
            self.xAxisRange = [0,80000];
            self.yAxisRange = [-2000,2000];

			self.numChannels = 16;
			self.sampleRate = 40000;
            
            plotTitle = 'Open Ephys Data Stream';
            xLabel = 'Sample Count';
            yLabel = 'Voltage [uV]';

            self.fig = gcf;

			clf; cla; 
			self.hPlot = plot(0,0); drawnow; hold on;

			title(plotTitle);
			xlabel(xLabel); ylabel(yLabel);
			xlim(self.xAxisRange); ylim(self.yAxisRange);

			self.process();

		end

    end

	methods (Access = protected)

		function process(self)
			xAxisSize = self.xAxisRange(2);
			plotUpdateInterval = 256; % Update plot every 256 iterations to reduce updates
			yOffsetIncrement = 200;  % Y offset increment
			baseYOffset = -1600;  % Base Y offset

			% Initialize xData and yData to maintain plot data continuity
			xData = 1:xAxisSize;
			yData = zeros(self.numChannels, xAxisSize);

			% Initialize the plots
			hold on;
			hPlots = gobjects(self.numChannels, 1);
			for i = 1:self.numChannels
				hPlots(i) = plot(xData, yData(i, :), 'DisplayName', sprintf('Function %d', i));
			end
			hold off;
			axis([1 xAxisSize baseYOffset baseYOffset + self.numChannels * yOffsetIncrement]);
			drawnow;

			numSamples = 0; % Default number of samples per fetch (should be updated below)
			startIdx = 1;
			iterationCounter = 0; % Counter for managing plot updates

			while true
				process@GenericProcessor(self); 
				numSamples = self.dataIn.numSamplesFetched; % update numSamples with actual fetched sample size
				currentRow = mod(iterationCounter, self.numChannels) + 1; % Determine the current row for plotting

				% Calculate new indices, ensuring they wrap around correctly
				endIdx = startIdx + numSamples - 1;
				if endIdx > xAxisSize
					endIdx = xAxisSize;
				end

				% Update the data for the current row
				yData(currentRow, startIdx:endIdx) = self.dataIn.continuous(1:(endIdx - startIdx + 1)) + (currentRow - 1) * yOffsetIncrement + baseYOffset;

				if mod(iterationCounter + 1, self.numChannels) == 0
					startIdx = endIdx + 1; % Update startIdx after 16 iterations
					if startIdx > xAxisSize
						startIdx = 1; % Wrap around the index
					end
				end

				% Increment iteration counter
				iterationCounter = iterationCounter + 1;

				% Update the plot less frequently
				if mod(iterationCounter, plotUpdateInterval) == 0
					for i = 1:self.numChannels
						set(hPlots(i), 'YData', yData(i, :));
					end
					drawnow limitrate; % Optimize drawing frequency
				end

				% Break condition on 'q' press
				if get(self.fig, 'CurrentCharacter') == 'q'
					break;
				end
			end
		end
	end

end