        set -o errexit
        set -o verbose

        if [[ ${disable_unit_tests|false} = "false" && ! -f ${skip_tests|/dev/null} ]]; then

        # activate the virtualenv if it has been set up
        ${activate_virtualenv}

        # Set the TMPDIR environment variable to be a directory in the task's working
        # directory so that temporary files created by processes spawned by resmoke.py get
        # cleaned up after the task completes. This also ensures the spawned processes
        # aren't impacted by limited space in the mount point for the /tmp directory.
        export TMPDIR="${workdir}/tmp"
        mkdir -p $TMPDIR

        if [ -f /proc/self/coredump_filter ]; then
          # Set the shell process (and its children processes) to dump ELF headers (bit 4),
          # anonymous shared mappings (bit 1), and anonymous private mappings (bit 0).
          echo 0x13 > /proc/self/coredump_filter

          if [ -f /sbin/sysctl ]; then
            # Check that the core pattern is set explicitly on our distro image instead
            # of being the OS's default value. This ensures that coredump names are consistent
            # across distros and can be picked up by Evergreen.
            core_pattern=$(/sbin/sysctl -n "kernel.core_pattern")
            if [ "$core_pattern" = "dump_%e.%p.core" ]; then
              echo "Enabling coredumps"
              ulimit -c unlimited
            fi
          fi
        fi

        if [ $(uname -s) == "Darwin" ]; then
            core_pattern_mac=$(/usr/sbin/sysctl -n "kern.corefile")
            if [ "$core_pattern_mac" = "dump_%N.%P.core" ]; then
              echo "Enabling coredumps"
              ulimit -c unlimited
            fi
        fi

        extra_args="$extra_args --jobs=${resmoke_jobs|1}"

        if [ ${should_shuffle|true} = true ]; then
          extra_args="$extra_args --shuffle"
        fi

        if [ ${continue_on_failure|true} = true ]; then
          extra_args="$extra_args --continueOnFailure"
        fi

        # We reduce the storage engine's cache size to reduce the likelihood of a mongod process
        # being killed by the OOM killer. The --storageEngineCacheSizeGB command line option is only
        # filled in with a default value here if one hasn't already been specified in the task's
        # definition or build variant's definition.
        set +o errexit
        echo "${resmoke_args} ${test_flags}" | grep -q storageEngineCacheSizeGB
        if [ $? -eq 1 ]; then
          echo "${resmoke_args} ${test_flags}" | grep -q "\-\-storageEngine=inMemory"
          if [ $? -eq 0 ]; then
            # We use a default of 4GB for the InMemory storage engine.
            extra_args="$extra_args --storageEngineCacheSizeGB=4"
          else
            # We use a default of 1GB for all other storage engines.
            extra_args="$extra_args --storageEngineCacheSizeGB=1"
          fi
        fi
        set -o errexit


        # Reduce the JSHeapLimit for the serial_run task task on Code Coverage builder variant.
        if [[ "${build_variant}" = "enterprise-rhel-62-64-bit-coverage" && "${task_name}" = "serial_run" ]]; then
          extra_args="$extra_args --mongodSetParameter {'jsHeapLimitMB':10}"
        fi

        path_value="$PATH"
        if [ ${variant_path_suffix} ]; then
          path_value="$path_value:${variant_path_suffix}"
        fi
        if [ ${task_path_suffix} ]; then
          path_value="$path_value:${task_path_suffix}"
        fi

        # The "resmoke_wrapper" expansion is used by the 'burn_in_tests' task to wrap the resmoke.py
        # invocation. It doesn't set any environment variables and should therefore come last in
        # this list of expansions.
        set +o errexit
        PATH="$path_value"                              \
            AWS_PROFILE=${aws_profile_remote}           \
            ${gcov_environment}                         \
            ${lang_environment}                         \
            ${san_options}                              \
            ${san_symbolizer}                           \
            ${snmp_config_path}                         \
            ${resmoke_wrapper}                          \
            $python buildscripts/evergreen_run_tests.py \
                ${resmoke_args}                         \
                $extra_args                             \
                ${test_flags}                           \
                --log=buildlogger                       \
                --staggerJobs=on                        \
                --buildId=${build_id}                   \
                --distroId=${distro_id}                 \
                --executionNumber=${execution}          \
                --projectName=${project}                \
                --gitRevision=${revision}               \
                --revisionOrderId=${revision_order_id}  \
                --taskId=${task_id}                     \
                --taskName=${task_name}                 \
                --variantName=${build_variant}          \
                --versionId=${version_id}               \
                --archiveFile=archive.json              \
                --reportFile=report.json                \
                --perfReportFile=perf.json
        resmoke_exit_code=$?
        set -o errexit

        # 74 is exit code for IOError on POSIX systems, which is raised when the machine is
        # shutting down.
        #
        # 75 is exit code resmoke.py uses when the log output would be incomplete due to failing
        # to communicate with logkeeper.
        if [[ $resmoke_exit_code = 74 || $resmoke_exit_code = 75 ]]; then
          echo $resmoke_exit_code > run_tests_infrastructure_failure
          exit 0
        elif [ $resmoke_exit_code != 0 ]; then
          # On failure save the resmoke exit code.
          echo $resmoke_exit_code > resmoke_error_code
        fi
        exit $resmoke_exit_code
        fi # end if [[ ${disable_unit_tests} && ! -f ${skip_tests|/dev/null} ]]