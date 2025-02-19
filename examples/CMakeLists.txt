cmake_minimum_required(VERSION 3.0...3.12)
project(open62541-examples C)
if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()
# This examples folder can also be built standalone.
# First install open62541 using `make install` then
# copy this folder to any other location and call CMake directly:
#
# cp -r open62541/examples $HOME/open62541_examples
# cd $HOME/open62541_examples
# mkdir build && cd build
# cmake -DUA_NAMESPACE_ZERO=FULL ..
# make -j

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # Examples are built standalone. Find installed open62541

    if(UA_NAMESPACE_ZERO STREQUAL "FULL")
        find_package(open62541 REQUIRED COMPONENTS FullNamespace)
    else()
        find_package(open62541 REQUIRED)
    endif()

    if(NOT UA_TOOLS_DIR)
        set(UA_TOOLS_DIR ${open62541_TOOLS_DIR})
    endif()

    function(assign_source_group)
        # define empty function. We don't need it in standalone
    endfunction(assign_source_group)

    include_directories(${PROJECT_BINARY_DIR}/src_generated)
endif()

# Required for common.h header file used in examples
include_directories(${CMAKE_CURRENT_LIST_DIR})

if(UA_ENABLE_AMALGAMATION)
    add_definitions(-DUA_ENABLE_AMALGAMATION)
endif()

#############################
# Compiled binaries folders #
#############################

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/examples)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/examples)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/examples)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/bin/examples)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/bin/examples)

macro(add_example EXAMPLE_NAME EXAMPLE_SOURCE)
  add_executable(${EXAMPLE_NAME} ${STATIC_OBJECTS} ${EXAMPLE_SOURCE} ${ARGN} ${PROJECT_SOURCE_DIR}/common.h)
  target_link_libraries(${EXAMPLE_NAME} open62541::open62541)
  assign_source_group(${EXAMPLE_SOURCE})
  set_target_properties(${EXAMPLE_NAME} PROPERTIES FOLDER "open62541/examples")
  set_target_properties(${EXAMPLE_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endmacro()

#############
# Tutorials #
#############

add_example(tutorial_datatypes tutorial_datatypes.c)

add_example(tutorial_server_firststeps tutorial_server_firststeps.c)

add_example(tutorial_server_variable tutorial_server_variable.c)

add_example(tutorial_server_datasource tutorial_server_datasource.c)

add_example(server_settimestamp server_settimestamp.c)

if(UA_ENABLE_SUBSCRIPTIONS)
    add_example(tutorial_server_monitoreditems tutorial_server_monitoreditems.c)
endif()

add_example(tutorial_server_variabletype tutorial_server_variabletype.c)

add_example(tutorial_server_object tutorial_server_object.c)

add_example(tutorial_server_reverseconnect tutorial_server_reverseconnect.c)

if(UA_ENABLE_METHODCALLS)
    add_example(tutorial_server_method tutorial_server_method.c)
    if (UA_MULTITHREADING GREATER_EQUAL 100)
        add_example(tutorial_server_method_async tutorial_server_method_async.c)
    endif()
endif()

add_example(tutorial_client_firststeps tutorial_client_firststeps.c)

if(UA_ENABLE_SUBSCRIPTIONS_EVENTS)
  add_example(tutorial_client_events tutorial_client_events.c)
  add_example(tutorial_server_events tutorial_server_events.c)
  add_example(server_events_random events/server_random_events.c)
  add_example(client_event_filter events/client_eventfilter.c)
  if(UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS)
    add_example(tutorial_server_alarms_conditions tutorial_server_alarms_conditions.c)
  endif()
endif()

##################
# Example Server #
##################

add_example(server_ctt server_ctt.c)

install(PROGRAMS $<TARGET_FILE:server_ctt>
        DESTINATION bin
        RENAME ua_server_ctt.exe)

##################
# Example Client #
##################

add_example(client client.c)

add_example(client_connect client_connect.c)

if(UA_ENABLE_HISTORIZING)
    add_example(client_historical client_historical.c)
endif()

install(PROGRAMS $<TARGET_FILE:client>
        DESTINATION bin
        RENAME ua_client)

add_example(client_async client_async.c)

if(UA_ENABLE_METHODCALLS)
    if (UA_MULTITHREADING GREATER_EQUAL 100)
        add_example(client_method_async client_method_async.c)
    endif()
endif()

add_example(client_connect_loop client_connect_loop.c)

if(UA_ENABLE_SUBSCRIPTIONS)
    add_example(client_subscription_loop client_subscription_loop.c)
endif()

####################
# Feature Examples #
####################

add_example(server_mainloop server_mainloop.c)

add_example(server_instantiation server_instantiation.c)

add_example(server_repeated_job server_repeated_job.c)

add_example(server_inheritance server_inheritance.c)

add_example(server_loglevel server_loglevel.c)

if(UA_ENABLE_HISTORIZING)
    add_example(tutorial_server_historicaldata tutorial_server_historicaldata.c)
    add_example(tutorial_server_historicaldata_circular tutorial_server_historicaldata_circular.c)
endif()

if(UA_ENABLE_ENCRYPTION OR UA_ENABLE_ENCRYPTION STREQUAL "MBEDTLS" OR UA_ENABLE_ENCRYPTION STREQUAL "OPENSSL")
    add_example(server_encryption encryption/server_encryption.c)
    add_example(client_encryption encryption/client_encryption.c)
    target_include_directories(server_encryption PRIVATE "${PROJECT_SOURCE_DIR}/examples")
    target_include_directories(client_encryption PRIVATE "${PROJECT_SOURCE_DIR}/examples")
    if(UA_ENABLE_TPM2_KEYSTORE)
        add_example(server_encryption_tpm_keystore encryption/server_encryption_tpm_keystore.c)
        add_example(client_encryption_tpm_keystore encryption/client_encryption_tpm_keystore.c)
        target_link_libraries(server_encryption_tpm_keystore tpm2_pkcs11 ssl crypto)
        target_link_libraries(client_encryption_tpm_keystore tpm2_pkcs11 ssl crypto)
    endif()
endif()

if (NOT (BUILD_SHARED_LIBS AND WIN32))
    add_example(custom_datatype_client custom_datatype/client_types_custom.c)
    add_example(custom_datatype_server custom_datatype/server_types_custom.c)
else()
    MESSAGE(WARNING "Can't build custom datatype examples on WIN32 when BUILD_SHARED_LIBS enabled. Skipping
custom_datatype_client and custom_datatype_server!")
endif()

if(UA_ENABLE_NODEMANAGEMENT)
    add_example(access_control_server access_control/server_access_control.c)
    add_example(access_control_client access_control/client_access_control.c)
    if(UA_ENABLE_ENCRYPTION OR UA_ENABLE_ENCRYPTION STREQUAL "MBEDTLS" OR UA_ENABLE_ENCRYPTION STREQUAL "OPENSSL")
    add_example(access_control_client_encrypt access_control_encrypt/client_access_control_encrypt.c)
    endif()
endif()

if(UA_ENABLE_DISCOVERY_MULTICAST)
    add_example(discovery_server_lds discovery/server_lds.c)
    add_example(discovery_server_register discovery/server_register.c)
    add_example(discovery_client_find_servers discovery/client_find_servers.c)
    add_example(discovery_server_multicast discovery/server_multicast.c)
endif()

add_subdirectory(nodeset)
####################
#  Example PubSub  #
####################

if(UA_ENABLE_PUBSUB)
    add_example(tutorial_pubsub_connection pubsub/tutorial_pubsub_connection.c)
    add_example(tutorial_pubsub_publish pubsub/tutorial_pubsub_publish.c)
    add_example(server_pubsub_publish_on_demand pubsub/server_pubsub_publisher_on_demand.c)
    add_example(tutorial_pubsub_publish_raw pubsub/tutorial_pubsub_publish_raw.c)
    add_example(tutorial_pubsub_subscribe_raw pubsub/tutorial_pubsub_subscribe_raw.c)
    add_example(server_pubsub_publisher_iop pubsub/server_pubsub_publisher_iop.c)
    add_example(server_pubsub_publish_rt_level pubsub/server_pubsub_publisher_rt_level.c)
    add_example(server_pubsub_publish_rt_level_raw pubsub/server_pubsub_publisher_rt_level_raw.c)
    add_example(server_pubsub_rt_information_model pubsub/server_pubsub_rt_field_information_model.c)
    add_example(pubsub_subscribe_standalone_dataset pubsub/pubsub_subscribe_standalone_dataset.c)
    if(CMAKE_SYSTEM MATCHES "Linux")
        if(UA_ENABLE_PUBSUB_ETH_UADP AND UA_ENABLE_MALLOC_SINGLETON AND UA_ENABLE_IMMUTABLE_NODES)
            add_example(rt_publisher pubsub_realtime/pubsub_interrupt_publish.c)
            target_link_libraries(rt_publisher rt)
        endif()
    endif()
    add_example(server_pubsub_subscribe_rt_level pubsub/server_pubsub_subscriber_rt_level.c)
    add_example(tutorial_pubsub_subscribe pubsub/tutorial_pubsub_subscribe.c)
    if (BUILD_SHARED_LIBS)
        message(WARNING "Build option BUILD_SHARED_LIBS not supported for standalone subscriber and realtime examples. Skipping these examples.")
    else (NOT BUILD_SHARED_LIBS)
        add_example(pubsub_subscribe_standalone pubsub/pubsub_subscribe_standalone.c)
        if(CMAKE_SYSTEM MATCHES "Linux")
            add_example(pubsub_TSN_publisher_multiple_thread pubsub_realtime/pubsub_TSN_publisher_multiple_thread.c)
            add_example(pubsub_TSN_loopback_single_thread pubsub_realtime/pubsub_TSN_loopback_single_thread.c)
            target_link_libraries(pubsub_TSN_publisher_multiple_thread rt pthread)
            target_link_libraries(pubsub_TSN_loopback_single_thread rt pthread)
            if(UA_ENABLE_PUBSUB_ETH_UADP)
                add_subdirectory(pubsub_realtime/nodeset)
                add_example(pubsub_TSN_publisher pubsub_realtime/pubsub_TSN_publisher.c)
                add_example(pubsub_TSN_loopback pubsub_realtime/pubsub_TSN_loopback.c)
                target_link_libraries(pubsub_TSN_publisher rt pthread)
                target_link_libraries(pubsub_TSN_loopback rt pthread)
            endif()
        endif()
    endif()
    if(UA_ENABLE_PUBSUB_ENCRYPTION)
        add_example(pubsub_publish_encrypted pubsub/pubsub_publish_encrypted.c)
        add_example(pubsub_subscribe_encrypted pubsub/pubsub_subscribe_encrypted.c)
        if(UA_ENABLE_TPM2_SECURITY)
            add_example(pubsub_publish_encrypted_tpm pubsub/pubsub_publish_encrypted_tpm.c)
            add_example(pubsub_subscribe_encrypted_tpm pubsub/pubsub_subscribe_encrypted_tpm.c)
        endif()
        if(UA_ENABLE_TPM2_KEYSTORE)
            add_example(pubsub_publish_encrypted_tpm_keystore pubsub/pubsub_publish_encrypted_tpm_keystore.c)
            add_example(pubsub_subscribe_encrypted_tpm_keystore pubsub/pubsub_subscribe_encrypted_tpm_keystore.c)
            target_link_libraries(pubsub_publish_encrypted_tpm_keystore tpm2_pkcs11 ssl crypto)
            target_link_libraries(pubsub_subscribe_encrypted_tpm_keystore tpm2_pkcs11 ssl crypto)
        endif()
        if(UA_ENABLE_PUBSUB_SKS)
        add_subdirectory(pubsub/sks)
        endif()
    endif()


    if(UA_ENABLE_PUBSUB_MQTT)
        if(NOT WIN32)
            include_directories(${PROJECT_SOURCE_DIR}/../plugins)
            add_example(tutorial_pubsub_mqtt_publish pubsub/tutorial_pubsub_mqtt_publish.c)
            add_example(tutorial_pubsub_mqtt_subscribe pubsub/tutorial_pubsub_mqtt_subscribe.c)
        endif()
    endif()
    if(UA_ENABLE_PUBSUB_FILE_CONFIG)
        add_example(server_pubsub_file_configuration pubsub/server_pubsub_file_configuration.c)
    endif()
    if (UA_ENABLE_PUBSUB_MONITORING)
        if(CMAKE_SYSTEM MATCHES "Linux")
            add_example(server_pubsub_subscribe_custom_monitoring pubsub/monitoring/server_pubsub_subscribe_custom_monitoring.c)
        else(NOT (CMAKE_SYSTEM MATCHES "Linux"))
            message(WARNING "PubSub subscriber monitoring example build needs LINUX OS. Skip build.")
        endif()
    endif()
endif()

###########################
# Nodeser Loader Examples #
###########################
if(UA_ENABLE_NODESETLOADER)
    add_subdirectory(nodeset_loader)
endif()
