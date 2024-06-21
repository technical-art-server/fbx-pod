﻿#include <args-parser/all.hpp>
#include <memory>
#include <filesystem>

#include <magic_enum/magic_enum.hpp>

#include "FlatBufferLoader.h"
#include "FBXLibra.h"
#include "CounterWeight/HierarchyCounterWeight.h"
#include "CounterWeight/HierarchyCounterWeightFactory.h"

using namespace Args;
namespace fs = std::filesystem;

int main(int argc, char ** argv) {
    try {
        CmdLine cmd(argc, argv, CmdLine::CommandIsRequired );
        Command check(SL("check"), ValueOptions::ManyValues, false);
        check.setDescription(SL("Check the fbx."));

        Arg fbx('f', SL("fbx"), true, true);
        check.addArg(&fbx);

        Command create(SL("create"), ValueOptions::ManyValues, false);
        create.addArg(&fbx);
//        Arg output('o', SL("output"), true, true);
//        create.addArg(&output);

        Help help;
        help.setAppDescription(SL("help"));
        help.setExecutable(argv[0]);

        cmd.addArg(&check);
        cmd.addArg(&create);
        cmd.addArg(help);

        cmd.parse();

        if (cmd.isDefined("check")){
            FBXLibraClient libraClient;

            for (const auto & value : cmd.values("check")){
                int ext_pos = value.find_last_of('.');
                if (ext_pos == std::string::npos){
                    throw BaseException("Invalid file path.");
                }
                std::string ext = value.substr(ext_pos,value.size()-ext_pos);
                // TODO: ここはファクトリークラスにする必要がある

                CounterWeight* weight;
                CounterWeight* fbx_weight;

                CounterWeightFactory* factory;
                if (ext == ".hcw"){
                    factory = new HierarchyCounterWeightFactory();
                    weight = factory->Load(value);
                    fbx_weight = factory->Create(cmd.value("-f"));
                }else{
                    throw BaseException("Invalid file extension.");
                }

                std::cout << "weight: " << value << std::endl;
                std::cout << "fbx path: " << cmd.value("-f") << std::endl;
                Status result = FBXLibraClient::Weigh(weight, fbx_weight);
                if (result == Status::SUCCESS){
                    std::cout << "\033[32m";
                } else{
                    std::cout << "\033[31m";
                }
                std::cout << "Hierarchy: " << magic_enum::enum_name(result) << std::endl;
                delete factory;
                delete weight;
                delete fbx_weight;
            }
        } else if (cmd.isDefined("create")){
            for (const auto & value : cmd.values("create")){
                fs::path output_file_path = fs::path(value);
                std::cout << output_file_path.extension() << std::endl;
                if (output_file_path.extension() == ".hcw"){
                    HierarchyCounterWeightFactory factory;
                    factory.Create(cmd.value("-f"));
                    HierarchyCounterWeight::Save(*factory.GetBuilder(), output_file_path);
                }else if (value == "vcw"){

                }else{
                    throw BaseException("Invalid file extension.");
                }
                std::cout << value << std::endl;
            }
            std::cout << cmd.value("-f") << std::endl;
        }
    }
    catch (const HelpHasBeenPrintedException &){
        return 0;
    }
    catch (const BaseException & e){
        std::cout << e.desc() << std::endl;
        return 1;
    }
    return 0;
}