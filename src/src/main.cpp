#include "optparse/optparse.hpp"
#include "spdlog/spdlog.h"

#include "SDVXPADServer.hpp"
#include "KeyboardSimulator.hpp"
#include "Utils.hpp"

#include "version.rc"

std::string banner = R"(
                                                                                                  
                                           ,-.----.                                
   ,---,   ,---,    ,---,    ,--,     ,--, \    /  \     ,---,           ,---,     
,`--.' |,`--.' |  .'  .' `\  |'. \   / .`| |   :    \   '  .' \        .'  .' `\   
|   :  :|   :  :,---.'     \ ; \ `\ /' / ; |   |  .\ : /  ;    '.    ,---.'     \  
:   |  ':   |  '|   |  .`\  |`. \  /  / .' .   :  |: |:  :       \   |   |  .`\  | 
|   :  ||   :  |:   : |  '  | \  \/  / ./  |   |   \ ::  |   /\   \  :   : |  '  | 
'   '  ;'   '  ;|   ' '  ;  :  \  \.'  /   |   : .   /|  :  ' ;.   : |   ' '  ;  : 
|   |  ||   |  |'   | ;  .  |   \  ;  ;    ;   | |`-' |  |  ;/  \   \'   | ;  .  | 
'   :  ;'   :  ;|   | :  |  '  / \  \  \   |   | ;    '  :  | \  \ ,'|   | :  |  ' 
|   |  '|   |  ''   : | /  ;  ;  /\  \  \  :   ' |    |  |  '  '--'  '   : | /  ;  
'   :  |'   :  ||   | '` ,/ ./__;  \  ;  \ :   : :    |  :  :        |   | '` ,/   
;   |.' ;   |.' ;   :  .'   |   : / \  \  ;|   | :    |  | ,'        ;   :  .'     
'---'   '---'   |   ,.'     ;   |/   \  ' |`---'.|    `--''          |   ,.'       
                '---'       `---'     `--`   `---`                   '---'                                                                                                           

=======================================================================
SDVX controller for pad output, by @AshenOneYe
IIDX version forked, by @wufe8, v)" VERSION_STRING;

std::string epilog = R"(
Open the URL displayed on a touch-enabled device (big iPads recommended)
connected to the same wifi access point as your windows machine.
Running a hotspot from your windows machine also works. Make sure
correct firewall access is granted.)";

int main(int argc, char **argv)
{
    optparse::OptionParser parser = optparse::OptionParser()
                                        .description(banner.substr(1))
                                        .version(VERSION_STRING)
                                        .epilog(epilog.substr(1));

    parser.add_option("-p", "--port").dest("port").type("int").set_default(1000).help("Port to listen on (1-65535)");
    parser.add_option("-f", "--frequency").dest("frequency").type("int").set_default(1000).help("Polling frequency, samples per second (1-1000)");
    parser.add_option("-d", "--dry-run").dest("dryrun").type("bool").set_default(false).action("store_true").help("Run server but do not send any keystrokes");
    parser.add_option("-q", "--quiet").dest("quiet").type("bool").set_default(false).action("store_true").help("Do not print any output");
    parser.add_option("-v", "--verbose").dest("verbose").type("bool").set_default(false).action("store_true").help("Print verbose output");
    parser.add_option("-m", "--mode").dest("mode").type("int").set_default(2).action("store_true").help("Select input key map(1: brokenithm, 2: TBW, 3: IIDX/BMS, 4: Debug)");

    const optparse::Values options = parser.parse_args(argc, argv);

    int port = static_cast<int>(options.get("port"));
    if (port < 1 || 65535 < port)
    {
        spdlog::error("Invalid port {}", port);
        exit(1);
    }

    int frequency = static_cast<int>(options.get("frequency"));
    if (frequency < 1 || 1000 < frequency)
    {
        spdlog::error("Invalid frequency {}", frequency);
        exit(1);
    }
    int millis_delay = std::clamp(1000 / frequency, 1, 1000);

    bool dryrun = static_cast<bool>(options.get("dryrun"));

    std::vector<std::string> ip_addresses = get_ip_addresses();
    if (ip_addresses.size() == 0)
    {
        spdlog::error("Cannot connect to network, no IP addresses found");
    }

    bool quiet = static_cast<bool>(options.get("quiet"));
    bool verbose = static_cast<bool>(options.get("verbose"));

    if (quiet && verbose)
    {
        spdlog::error("Cannot use quiet and verbose mode at the same time");
    }

    if (quiet)
    {
        spdlog::set_level(spdlog::level::off);
    }
    else if (verbose)
    {
        spdlog::set_level(spdlog::level::debug);
    }
    else
    {
        spdlog::set_level(spdlog::level::info);
    }

    if (!quiet)
    {
        std::cout << banner << std::endl;

        std::cout << "Opening IIDXPAD server at:\n";
        for (auto ip_address : ip_addresses)
        {
            std::cout << "http://" << ip_address << ":" << port << "/\n";
        }
        std::cout << std::flush;
    }

    SDVXPADServer sdvxpadServer(port);
    sdvxpadServer.start_server();

    KeyboardSimulatorLayout mode = KeyboardSimulatorLayout(static_cast<int>(options.get("mode")));
    KeyboardSimulator keyboardSimulator(mode);


    while (1)
    {
        keyboardSimulator.delay(millis_delay);
        if (!dryrun)
        {
            keyboardSimulator.send(sdvxpadServer.get_controller_state());
        }
    }
}
