#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <windows.h>
#include <unit-test/bdd-for-c.h>
#include "../source/cfabric.h"

spec("Testing cfabric")
{

    static cfb_plugin_t **v_class_factory;

    after()
    {
        printf("  All done!\n");
    }

    after()
    {
        //printf("%i tests run. %i failed.\n", test_count, tests_failed);
    }

    before()
    {
        //test_count = 0;
        //tests_failed = 0;
    }

    before()
    {
        printf("  Starting the tests...\n\n");
    }

    before_each()
    {
        //++test_count;
        //++tests_failed;
    }

    after_each()
    {
        //--tests_failed;
    }

    it("test cfb_define_plugin")
    {
        load_cfabric(cfb_define_plugin);
        cfb_init_all_plugins();
    }
}