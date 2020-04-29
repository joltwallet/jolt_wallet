import os
import os.path as osp
import importlib.util
from pprint import pprint

def add_unit_test_app_extension(base_actions, project_path=os.getcwd()):
    _spec = importlib.util.spec_from_file_location("idf_ext", osp.join(os.environ["IDF_PATH"], "tools", "unit-test-app", "idf_ext.py"))
    _unit_test_app_idf_ext = importlib.util.module_from_spec(_spec)
    _spec.loader.exec_module(_unit_test_app_idf_ext)
    extensions = _unit_test_app_idf_ext.action_extensions(base_actions, project_path=project_path)
    return extensions

def action_extensions(base_actions, project_path=os.getcwd()):
    # Get the generic `build_target(target_name, ctx, args)`
    build_target = base_actions["actions"]["all"]["callback"]

    # Add the unit-testing-app extension
    extensions = add_unit_test_app_extension(base_actions, project_path=project_path)

    # All functions under extensions['global_action_callbacks']
    # are always executed
    def callback(ctx, global_args, tasks):
        #if 'define_cache_entry' not in global_args:
        #    global_args.define_cache_entry = []
        pass
    extensions['global_action_callbacks'].append(callback)

    # Functions registered under extensions["actions"]["my_command_name"]
    # are invoked when `idf.py my_command_name` is ran
    def build_test(build_name, ctx, args):
        #args['no_warnings'] = True

        # Always add the jolt_os test component
        # This UNIT_TESTING is now available to CMake as a variable,
        # but it must be parsed and passed along by the CMakeLists.txt 
        # to the actual build process.
        args['define_cache_entry'].extend([
            'TEST_ALL=0', 'TEST_COMPONENTS=jolt_os', 'UNIT_TESTING=1',
            ])
        build_target("all", ctx, args)

    def build_compress(build_name, ctx, args):
        build_target(build_name, ctx, args)

    extensions["actions"]["tests"] = {
        "callback": build_test,
    }
    extensions["actions"]["compress"] = {
        "callback": build_compress,
    }

    return extensions

