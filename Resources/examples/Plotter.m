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
		loop_count;
	end

    methods 

    	function self = Plotter(host, port)
            
            self = self@GenericProcessor(host, port);
            
            self.xAxisRange = [0,80000];
            self.yAxisRange = [-1000 1000];
            
            plotTitle = 'Open Ephys Data Stream';
            xLabel = 'Sample Count';
            yLabel = 'Voltage [uV]';

            self.fig = gcf;

			clf; cla; 
			self.hPlot = plot(0,0); drawnow; hold on;

			title(plotTitle);
			xlabel(xLabel); ylabel(yLabel);
			xlim(self.xAxisRange); ylim(self.yAxisRange);

			self.loop_count = 0;
			self.process();

		end

    end

	methods (Access = protected)

		function process(self)
            
			lastSample = 0;
            xAxisSize = self.xAxisRange(2);
            
			while 1

				process@GenericProcessor(self); 

				numSamples = self.dataIn.numSamplesFetched;
                
				%fprintf('Num samples fetched: %d\n', numSamples);

				if lastSample + numSamples > xAxisSize
					xData = (lastSample+1):xAxisSize;
					yData = self.dataIn.continuous(1:(xAxisSize-lastSample)) + mod(self.loop_count, 2)*1000;
				else
					xData = (lastSample+1):(lastSample+numSamples);
					yData = self.dataIn.continuous + mod(self.loop_count, 2)*1000;
				end

				self.hPlot = plot(xData,yData); drawnow;
				set(self.hPlot, 'XData', xData, 'YData', yData); 

				self.loop_count = self.loop_count + 1;

				lastSample = lastSample + mod(self.loop_count, 2)*numSamples;

				%TODO: Currently ignores (doesn't plot) samples that overshoot x-axis range
                if lastSample > xAxisSize
                    lastSample = 0;
                    cla; self.hPlot = plot(0,0); drawnow; 
                end

                if get(self.fig, 'CurrentCharacter') == 'q'
                    break;
                end

            end

		end

	end

end