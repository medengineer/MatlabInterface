classdef OEClient < handle
    
    %OECLIENT A client socket for receiving data streams from the Open-Ephys GUI. 
    %   This class encapsulates a socket on a dedicated, local port in order to
    %   receive continous data stream from Open Ephys. 

    properties (Access = public)
        header;
    end
    
    properties (Access = private)
        host;
        port;
        connected;
        socket;
        in_stream;
        out_stream;
        stream_writer;
        stream_reader;
        buffered_reader;
    end

    properties (Constant)
        END_OF_MESSAGE = '~';
        MAX_CONNECT_ATTEMPTS = 5;
        WRITE_MSG_SIZE = 5;
    end

    methods
        
        function self = OEClient(host, port)
            
            self.host = host;
            self.port = port;
            
            self.connected = 0;
            
            attempts = 0;
            while ~self.connected && attempts < self.MAX_CONNECT_ATTEMPTS
                self.connect(host,port);
                attempts = attempts + 1;
            end
            
            if ~self.connected 
                fprintf("Failed to connect to Open Ephys!\n");
            end
            
        end

        function line = read(self)

            line = self.buffered_reader.readLine;

        end

        function self = write(self, message)

            %TODO: Check if message is a valid char and proper length

            if (length(message) == self.WRITE_MSG_SIZE)
                self.stream_writer.writeBytes(message);
                self.stream_writer.flush;
            end

        end
        
        function self = connect(self, host, port)
            
            import java.net.Socket
            import java.util.Scanner
            import java.io.*
            
            try
                self.socket = Socket(host, port);
                fprintf("Created a new socket on %s:%d\n", self.host, self.port);
                self.connected = 1;
                self.in_stream = self.socket.getInputStream;
                fprintf("Created input stream...\n");
                self.out_stream = self.socket.getOutputStream;
                fprintf("Created output stream...\n");
                self.stream_writer = DataOutputStream(self.out_stream);
                fprintf("Created output stream writer...\n");
                self.stream_reader = InputStreamReader(self.in_stream);
                fprintf("Created input stream reader...\n");
                self.buffered_reader = BufferedReader(self.stream_reader);
                fprintf("Created buffered reader...\n");
            catch 
                fprintf("Connecting...\n");
            end
           
        end
        
    end
    
end

