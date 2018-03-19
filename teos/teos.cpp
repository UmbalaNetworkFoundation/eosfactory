#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>

#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>

#include <teoslib/command/get_commands.hpp>
#include <teoslib/command/wallet_commands.hpp>
#include <teoslib/command/create_commands.hpp>
#include <teoslib/command/set_commands.hpp>
#include <teoslib/command/push_commands.hpp>
#include <teoslib/command/other_commands.hpp>
#include <teoslib/command/subcommands.hpp>

#include <teoslib/control/daemon_controls.hpp>
#include <teoslib/control/config.hpp>

#include <teos/teos.hpp>

#define IF_ELSE(commandName_, classPrefix) \
  if (commandName == #commandName_) \
  { \
    classPrefix##Options(argcLeft, argvLeft).go(); \
    for( int i = 0; i < argcLeft; i++ ){ \
      delete[] argvLeft[i]; \
    } \
    delete[] argvLeft; \
  } \
  else

#define HELP                  \
  std::cout << usage << endl; \
  std::cout << desc << endl;  \
  std::cout << commands << endl;

const char* usage = R"EOF(
Command Line Interface to Eos Daemon
Usage: ./teos [HOST:PORT] [OPTIONS] [COMMAND] [SUBCOMMAND] [OPTIONS]
for example:
192.168.229.140:8888 get block 255
)EOF";

const char* commands = R"EOF(
Commands:
  create      Create various items, on and off the blockchain
  get         Retrieve various items and information from the blockchain
  set         Set or update blockchain state
  transfer    Transfer EOS from account to account
  wallet      Interact with local wallet
  benchmark   Configure and execute benchmarks
  push        Push arbitrary transactions to the blockchain
  node        Test EOS chain node procedures
)EOF";

std::map<const std::string, const std::string> subcommandMap = {
  { "create", createSubcommands },
  { "get", getSubcommands },
  { "set", setSubcommands },
  { "wallet", walletSubcommands },
  { "benchmark", benchmarkSubcommands },
  { "push", pushSubcommands }
};

#ifdef WIN32
extern "C" FILE*  __cdecl __iob_func(void);
#endif // WIN32

int main(int argc, const char *argv[]) {
#ifdef WIN32
  __iob_func();
#endif // WIN32

  using namespace std;
  using namespace teos;
  using namespace teos::command;
  using namespace teos::control;
  using namespace boost::program_options;

  const char* argv0 = argv[0];
  int argcLeft;
  const char** argvLeft;

  options_description desc{ "Options" };
  string command;
  string subcommand;

  if (argc > 1)
  {
    string ipAddress(argv[1]);
    size_t colon = ipAddress.find(":");
    if (colon != std::string::npos)
    {
      TeosCommand::httpAddress = ipAddress;
      TeosCommand::httpWalletAddress = TeosCommand::httpAddress;
      argv++;
      argc--;
    }

    if (strcmp(argv[1], "tokenika") == 0)
    {
      TeosCommand::httpAddress = TEST_HTTP_ADDRESS;
      TeosCommand::httpWalletAddress = TeosCommand::httpAddress;
      argv++;
      argc--;
    }
  }

  try
  {
    options_description desc{ "Options" };
    desc.add(CommandOptions::httpOptions());
    desc.add_options()
      ("help,h", "Help screen")
      ("verbose,V", "Output verbose messages on error");

    command_line_parser parser{ argc, argv };
    parser.options(desc).allow_unregistered();
    parsed_options parsed_options = parser.run();

    vector<string> to_pass_further = collect_unrecognized(
      parsed_options.options, include_positional);

    variables_map vm;
    store(parsed_options, vm);
    notify(vm);

    if (vm.count("address"))
      TeosCommand::httpAddress = string(vm["address"].as<string>());
    if (vm.count("wallet"))
      TeosCommand::httpWalletAddress = string(vm["wallet"].as<string>());

    if (to_pass_further.size() > 0)
      command = to_pass_further[0];

    if (to_pass_further.size() > 1)
    {
      subcommand = to_pass_further[1];
      to_pass_further.erase(to_pass_further.begin(), to_pass_further.begin() + 2);
    } else
    {
      if (subcommandMap.find(command) != subcommandMap.end())
      {
        cout << subcommandMap.at(command) << endl;
        return(0);
      }
    }

    to_pass_further.insert(to_pass_further.begin(), argv0);    
    if (vm.count("help"))
      to_pass_further.push_back("-h");
    if (vm.count("verbose"))
      to_pass_further.push_back("-V");

    { // Convert to_pass_further std::vector to char** arr:
      argcLeft = (int)to_pass_further.size();
      char** arr = new char*[argcLeft];
      for (size_t i = 0; i < to_pass_further.size(); i++) {
        arr[i] = new char[to_pass_further[i].size() + 1];

#ifdef _MSC_VER
        strcpy_s(arr[i], to_pass_further[i].size() + 1,
          to_pass_further[i].c_str()); 
#else
        strcpy(arr[i], to_pass_further[i].c_str());
#endif
      }
      argvLeft = (const char**)arr;
    }

    if (vm.count("help") && command == "")
    {
      HELP
        return 0;
    }
  }
  catch (const boost::program_options::error &ex)
  {
    std::cerr << ex.what() << '\n';
    exit(-1);
  }

  if (command == "")
  {
    HELP
      return 0;
  }
  else
  {
    if (subcommand != "")
    {
      string commandName = command + "_" + subcommand;

      IF_ELSE(version_client, VersionClient)
      IF_ELSE(get_info, GetInfo)
      IF_ELSE(get_block, GetBlock)
      IF_ELSE(get_account, GetAccount)
      IF_ELSE(get_code, GetCode)
      IF_ELSE(get_table, GetTable)
      IF_ELSE(wallet_create, WalletCreate)
      IF_ELSE(wallet_list, WalletList)
      IF_ELSE(wallet_keys, WalletKeys)
      IF_ELSE(wallet_import, WalletImport)
      IF_ELSE(wallet_open, WalletOpen)
      IF_ELSE(wallet_lock, WalletLock)
      IF_ELSE(wallet_lock_all, WalletLockAll)
      IF_ELSE(wallet_unlock, WalletUnlock)
      IF_ELSE(create_key, CreateKey)
      IF_ELSE(create_account, CreateAccount)
      IF_ELSE(set_contract, SetContract)
      IF_ELSE(push_action, PushAction)
      IF_ELSE(daemon_start, DaemonStart)
      IF_ELSE(daemon_stop, DaemonStop)
      IF_ELSE(daemon_delete_wallets, DaemonDeleteWallets)
      IF_ELSE(config_teos, ConfigTeos)
      {
        cerr << "unknown command!" << endl;
      }
    }
    else
    {
      HELP
        return 0;
    }  
  }
  return 0;
}
