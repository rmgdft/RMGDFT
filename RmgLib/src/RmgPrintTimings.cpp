
#include <sys/time.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <functional>
#include <string>
#include <iomanip>
#include <algorithm>
#include "RmgTimer.h"


void RmgPrintTimings(BaseGrid *G, const char *outfile, int steps) {

    int tid;
    size_t i, count, count1;
    BaseThread *T = BaseThread::getBaseThread(0);
    RmgTimer RT("Print timings");

    std::ofstream logfile;

    if(G->get_rank() == 0) {

        logfile.open(outfile, std::ofstream::out | std::ofstream::app);

        // Have to do some manipulations to compute thread min/max/average and get things properly sorted
        std::map <std::string, double> tmain;
        std::map <std::string, double> tmin;
        std::map <std::string, double> tmax;
        std::map <std::string, double> tavg;

        // Reset main thread (tid=0) into an ordered map
        for(auto it = RT.timings[0].cbegin(); it != RT.timings[0].cend(); ++it) {
            tmain[it->first] = it->second;
        }

        size_t maxlen = 0;
        for(auto it = RT.timings[1].cbegin(); it != RT.timings[1].cend(); ++it) {
            tmin[it->first] = 99999999999.0;
            tmax[it->first] = 0.0;
            tavg[it->first] = 0.0;
            if(maxlen < it->first.length()) maxlen = it->first.length();
        }
        maxlen += 2;

        for(tid=1;tid <= T->get_threads_per_node();tid++) {
            for(auto it = RT.timings[tid].cbegin(); it != RT.timings[tid].cend(); ++it) {
                if(it->second < tmin[it->first]) tmin[it->first] = it->second;
                if(it->second > tmax[it->first]) tmax[it->first] = it->second;
                tavg[it->first] += it->second;
            }
        }

        logfile << std::endl << std::endl;
        logfile << std::fixed << std::setprecision(2);
        logfile << "------------------------- TIMING INFORMATION FOR MAIN  ----------------------" << std::endl;
        logfile << "                                                 Total time       Per SCF/step" << std::endl;
        count1 = 0;
        for(auto it = tmain.cbegin(); it != tmain.cend(); ++it) {
            count = std::count(it->first.begin(), it->first.end(), ':');  

            if(count1 < count) {
                for(i = 0; i < count1; i++) logfile << "  ";
                for(i = 2 * count1; i < 77; i++) logfile <<"-";
                logfile <<  std::endl;
            }

            if(count == 0) logfile << std::endl;

            for(i = 0; i < count; i++) logfile << "  ";
            logfile << std::setw(41-count*2) << std::left << it->first << std::setw(18) << std::right << it->second << std::setw(18) << std::right << it->second/(double)steps << std::endl;
                
            count1 = count;
        }

        logfile << std::endl << std::endl;
        logfile << "------------------------- TIMING INFORMATION FOR THREADS  -------------------" << std::endl << std::endl;
        logfile << "                                           Min            Max            Avg";

        auto it1 = tmin.cbegin();
        auto it2 = tmax.cbegin();
        auto it3 = tavg.cbegin();
        while(it1 != tmin.cend()) {
            std::size_t found = it1->first.find_first_of(":");
            if(found != std::string::npos) {
                logfile << "  ";
                logfile << std::setw(maxlen) << std::left << it1->first
                << std::setw(16) << std::right << it1->second
                << std::setw(15) << std::right << it2->second
                << std::setw(15) << std::right
                << it3->second/T->get_threads_per_node() << std::endl;
            }
            else {
                logfile << std::endl;
                logfile << std::setw(maxlen) << std::left << it1->first
                << std::setw(18) << std::right << it1->second
                << std::setw(15) << std::right << it2->second
                << std::setw(15) << std::right << it3->second/T->get_threads_per_node()
                << std::endl
                << "-----------------------------------------------------------------------------"
                << std::endl;
            }
            it1++;
            it2++;
            it3++;
        }

        logfile.close();

    }

}
