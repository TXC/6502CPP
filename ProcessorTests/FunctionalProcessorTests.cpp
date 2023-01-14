#include <catch2/catch_all.hpp>
#include <cstring>
#include "Bus.hpp"
#include "Processor.hpp"

using namespace CPU;

char datadir[20] = "../Functional Tests";
char functional_test_filename[30] = "/6502_functional_test.bin";
char interrupt_test_filename[30] = "/6502_interrupt_test.bin";
char cycle_test_filename[30] = "/6502_cycle_test.bin";

FILE* functional_test_file = fopen(strcat(datadir, functional_test_filename), "r");
FILE* interrupt_test_file = fopen(strcat(datadir, interrupt_test_filename), "r");
FILE* cycle_test_file = fopen(strcat(datadir, cycle_test_filename), "r");

TEST_CASE("Klaus Dorman Interrupt Test")
{

}

TEST_CASE("Klaus Dorman Functional Test")
{

}

TEST_CASE("Cycle Test")
{

}
