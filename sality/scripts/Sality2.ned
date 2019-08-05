package sality.ned_files;

import sality.ned_files.Superpeer;
import sality.ned_files.Crawler;
import sality.ned_files.Botmaster;

        network Sality
        {
        	types:
        		channel Channel extends ned.DelayChannel
        		{
        			delay = 50ms;
        		}
        	submodules:
        		peer[10]: Superpeer;
        		crawler: Crawler;
        		botmaster: Botmaster;
        	connections:
        		peer[0].outputGate++ <--> Channel <--> peer[1].inputGate++;
        		peer[9].outputGate++ <--> Channel <--> peer[4].inputGate++;
        		
        		crawler.gate++ <--> Channel <--> peer[0].inputGate++;
        		crawler.gate++ <--> Channel <--> peer[9].inputGate++;
        		
        		botmaster.gate++ <--> Channel <--> peer[0].inputGate++;
        		botmaster.gate++ <--> Channel <--> peer[9].inputGate++;
        }
