set -o errexit
set -o verbose

env

if [ "${EB_X_is_patch}" = "true" ] && [ "${EB_X_bypass_compile}" = "true" ]; then
  exit 0
fi
rm -rf ${install_directory}

extra_args=""
if [ -n "${num_scons_link_jobs_available}" ]; then
  echo "Changing SCons to run with --jlink=${num_scons_link_jobs_available}"
  extra_args="$extra_args --jlink=${num_scons_link_jobs_available}"
fi

if [ "${EB_X_scons_cache_scope}" = "shared" ]; then
  extra_args="$extra_args --cache-debug=scons_cache.log"
fi


# Enable performance debugging
extra_args="$extra_args --debug=time"

# If we are doing a patch build or we are building a non-push
# build on the waterfall, then we don't need the --release
# flag. Otherwise, this is potentially a build that "leaves
# the building", so we do want that flag. The non --release
# case should auto enale the faster decider when
# applicable. Furthermore, for the non --release cases we can
# accelerate the build slightly for situations where we invoke
# SCons multiple times on the same machine by allowing SCons
# to assume that implicit dependencies are cacheable across
# runs.
if [ "${EB_X_is_patch}" = "true" ] || [ -z "${EB_X_push_bucket}" ] || [ "${EB_X_compiling_for_test}" = "true" ]; then
  extra_args="$extra_args --implicit-cache --build-fast-and-loose=on"
else
  extra_args="$extra_args --release"
fi

${compile_env} python ./buildscripts/scons.py                                     \
    ${compile_flags} ${scons_cache_args} ${extra_args} ${targets} MONGO_VERSION=${EB_X_version} || exit_status=$?
# If compile fails we do not run any tests
if [[ $exit_status -ne 0 ]]; then
  if [[ "${dump_scons_config_on_failure}" == true ]]; then
    echo "Dumping build/scons/config.log"
    cat build/scons/config.log
  fi
  touch ${skip_tests}
fi
exit $exit_status