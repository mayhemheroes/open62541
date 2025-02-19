/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *    Copyright 2015-2016 (c) Sten Grüner
 *    Copyright 2015-2017 (c) Fraunhofer IOSB (Author: Julius Pfrommer)
 *    Copyright 2015 (c) Oleksiy Vasylyev
 *    Copyright 2016-2017 (c) Florian Palm
 *    Copyright 2017 (c) Stefan Profanter, fortiss GmbH
 *    Copyright 2017 (c) Mark Giraud, Fraunhofer IOSB
 */

#ifndef UA_CLIENT_INTERNAL_H_
#define UA_CLIENT_INTERNAL_H_

#define UA_INTERNAL
#include <open62541/client.h>
#include <open62541/client_highlevel.h>
#include <open62541/client_subscriptions.h>

#include "open62541_queue.h"
#include "ua_securechannel.h"
#include "common/ua_timer.h"
#include "ua_util_internal.h"
#include "ziptree.h"

_UA_BEGIN_DECLS

/**************************/
/* Subscriptions Handling */
/**************************/

#ifdef UA_ENABLE_SUBSCRIPTIONS

typedef struct UA_Client_NotificationsAckNumber {
    LIST_ENTRY(UA_Client_NotificationsAckNumber) listEntry;
    UA_SubscriptionAcknowledgement subAck;
} UA_Client_NotificationsAckNumber;

struct UA_Client_MonitoredItem;
typedef struct UA_Client_MonitoredItem UA_Client_MonitoredItem;

struct UA_Client_MonitoredItem {
    ZIP_ENTRY(UA_Client_MonitoredItem) zipfields;
    UA_UInt32 monitoredItemId;
    UA_UInt32 clientHandle;
    void *context;
    UA_Client_DeleteMonitoredItemCallback deleteCallback;
    union {
        UA_Client_DataChangeNotificationCallback dataChangeCallback;
        UA_Client_EventNotificationCallback eventCallback;
    } handler;
    UA_Boolean isEventMonitoredItem; /* Otherwise a DataChange MoniitoredItem */
};

ZIP_HEAD(MonitorItemsTree, UA_Client_MonitoredItem);
typedef struct MonitorItemsTree MonitorItemsTree;

typedef struct UA_Client_Subscription {
    LIST_ENTRY(UA_Client_Subscription) listEntry;
    UA_UInt32 subscriptionId;
    void *context;
    UA_Double publishingInterval;
    UA_UInt32 maxKeepAliveCount;
    UA_Client_StatusChangeNotificationCallback statusChangeCallback;
    UA_Client_DeleteSubscriptionCallback deleteCallback;
    UA_UInt32 sequenceNumber;
    UA_DateTime lastActivity;
    MonitorItemsTree monitoredItems;
} UA_Client_Subscription;

struct UA_Client_MonitoredItem_ForDelete {
    UA_Client *client;
    UA_Client_Subscription *sub;
    UA_UInt32 *monitoredItemId;
};

void
__Client_Subscriptions_clean(UA_Client *client);

/* Exposed for fuzzing */
UA_StatusCode
__Client_preparePublishRequest(UA_Client *client, UA_PublishRequest *request);

void
__Client_Subscriptions_backgroundPublish(UA_Client *client);

void
__Client_Subscriptions_backgroundPublishInactivityCheck(UA_Client *client);

#endif /* UA_ENABLE_SUBSCRIPTIONS */

/**********/
/* Client */
/**********/

typedef struct AsyncServiceCall {
    LIST_ENTRY(AsyncServiceCall) pointers;
    UA_UInt32 requestId;
    UA_ClientAsyncServiceCallback callback;
    const UA_DataType *responseType;
    void *userdata;
    UA_DateTime start;
    UA_UInt32 timeout;
    UA_Response *syncResponse; /* If non-null, then this is the synchronous
                                * response to be filled. Set back to null to
                                * indicate that the response was filled. */
} AsyncServiceCall;

typedef LIST_HEAD(UA_AsyncServiceList, AsyncServiceCall) UA_AsyncServiceList;

void
__Client_AsyncService_removeAll(UA_Client *client, UA_StatusCode statusCode);

typedef struct CustomCallback {
    UA_UInt32 callbackId;

    UA_ClientAsyncServiceCallback userCallback;
    void *userData;

    void *clientData;
} CustomCallback;

struct UA_Client {
    UA_ClientConfig config;

    /* Callback ID to remove it from the EventLoop */
    UA_UInt64 houseKeepingCallbackId;

    /* Overall connection status */
    UA_StatusCode connectStatus;

    /* Old status to notify only changes */
    UA_SecureChannelState oldChannelState;
    UA_SessionState oldSessionState;
    UA_StatusCode oldConnectStatus;

    UA_Boolean findServersHandshake;   /* Ongoing FindServers */
    UA_Boolean endpointsHandshake;     /* Ongoing GetEndpoints */
    UA_Boolean noSession;              /* Don't open a session */
    UA_Boolean noReconnect;            /* Don't reconnect when the connection closes */

    /* Connection */
    UA_String endpointUrl;  /* Used to extract address and port */
    UA_String discoveryUrl; /* The discoveryUrl (also used to signal which
                               application we want to connect to in the HEL/ACK
                               handshake). */

    /* SecureChannel */
    UA_SecureChannel channel;
    UA_UInt32 requestId;
    UA_DateTime nextChannelRenewal;

    /* Session */
    UA_SessionState sessionState;
    UA_NodeId authenticationToken;
    UA_UInt32 requestHandle;
    UA_ByteString remoteNonce;
    UA_ByteString localNonce;

    /* Connectivity check */
    UA_DateTime lastConnectivityCheck;
    UA_Boolean pendingConnectivityCheck;

    /* Async Service */
    UA_AsyncServiceList asyncServiceCalls;

    /* Subscriptions */
#ifdef UA_ENABLE_SUBSCRIPTIONS
    LIST_HEAD(, UA_Client_NotificationsAckNumber) pendingNotificationsAcks;
    LIST_HEAD(, UA_Client_Subscription) subscriptions;
    UA_UInt32 monitoredItemHandles;
    UA_UInt16 currentlyOutStandingPublishRequests;
#endif

    /* Internal locking for thread-safety. Methods starting with UA_Client_ that
     * are marked with UA_THREADSAFE take the lock. The lock is released before
     * dropping into the EventLoop and before calling user-defined callbacks.
     * That way user-defined callbacks can themselves call thread-safe client
     * methods. */
#if UA_MULTITHREADING >= 100
    UA_Lock clientMutex;
#endif
};

UA_StatusCode
__Client_AsyncService(UA_Client *client, const void *request,
                      const UA_DataType *requestType,
                      UA_ClientAsyncServiceCallback callback,
                      const UA_DataType *responseType,
                      void *userdata, UA_UInt32 *requestId);

void
__Client_Service(UA_Client *client, const void *request,
                 const UA_DataType *requestType, void *response,
                 const UA_DataType *responseType);

UA_StatusCode
__UA_Client_startup(UA_Client *client);

UA_StatusCode
__Client_renewSecureChannel(UA_Client *client);

UA_StatusCode
processServiceResponse(void *application, UA_SecureChannel *channel,
                       UA_MessageType messageType, UA_UInt32 requestId,
                       UA_ByteString *message);

UA_Boolean isFullyConnected(UA_Client *client);
void connectSync(UA_Client *client);
void notifyClientState(UA_Client *client);
void processRHEMessage(UA_Client *client, const UA_ByteString *chunk);
void processERRResponse(UA_Client *client, const UA_ByteString *chunk);
void processACKResponse(UA_Client *client, const UA_ByteString *chunk);
void processOPNResponse(UA_Client *client, const UA_ByteString *message);
void closeSecureChannel(UA_Client *client);
void cleanupSession(UA_Client *client);

void
Client_warnEndpointsResult(UA_Client *client,
                           const UA_GetEndpointsResponse *response,
                           const UA_String *endpointUrl);

_UA_END_DECLS

#endif /* UA_CLIENT_INTERNAL_H_ */
