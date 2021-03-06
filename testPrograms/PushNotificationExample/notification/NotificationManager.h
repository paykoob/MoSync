/*
Copyright (C) 2011 MoSync AB

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License,
version 2, as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
MA 02110-1301, USA.
*/

/**
 * @file NotificationManager.h
 * @author Bogdan Iusco
 *
 * \brief  The NotificationManager manages local and push notification events
 * and dispatches them to the target notifications.
 */

#ifndef NOTIFICATION_NOTIFICATION_MANAGER_H_
#define NOTIFICATION_NOTIFICATION_MANAGER_H_

#include <ma.h>

#include <MAUtil/String.h>
#include <MAUtil/Map.h>
#include <MAUtil/Environment.h>

namespace Notification
{

    /**
     * Constants indicating the types of push notifications the application
     * accepts.
     * Platform: iOS only.
     */
    enum PushNotificationType
    {
        // The application accepts notifications that badge the application icon.
        PUSH_NOTIFICATION_TYPE_BADGE = 0x01,
        // The application accepts alert sounds as notifications.
        PUSH_NOTIFICATION_TYPE_SOUND = 0x02,
        // The application accepts alert messages as notifications.
        PUSH_NOTIFICATION_TYPE_ALERT = 0x04
    };

    // Forward declaration.
    class LocalNotification;
    class PushNotificationListener;

    /**
     * \brief Class that handles notification events.
     */
    class NotificationManager : public MAUtil::CustomEventListener
    {
    public:
        /**
         * Destructor.
         */
        virtual ~NotificationManager();

        /**
         * Return the single instance of this class.
         */
        static NotificationManager* getInstance();

        /**
         * Destroy the single instance of this class.
         * Call this method only when the application will exit.
         */
        static void destroyInstance();

        /**
         * Implementation of CustomEventListener interface.
         * This method will get called whenever there is a
         * event generated by the system.
         * @param event The new received event.
         */
        virtual void customEvent(const MAEvent& event);

        /**
         * Add a local notification to the map that holds notifications.
         * The local notification will receive custom events.
         * @param localNotification The local notification that needs to be
         * registered.
         * The ownership of the local notification is not passed to this method.
         */
        virtual void addEventListener(
            LocalNotification* localNotification);

        /**
         * Remove a local notification from the map that holds notifications.
         * The local notification will not receive custom events.
         * @param banner The local notification that needs to be unregistered.
         */
        virtual void removeEventListener(
            LocalNotification* localNotification);

        /**
         * Schedules a local notification for delivery at its encapsulated
         * date and time.
         * @param localNotification Handle to a local notification object.
         */
        virtual void registerLocalNotification(
            LocalNotification* localNotification);

        /**
         * Cancels the delivery of the specified scheduled local notification.
         * calling this method also programmatically dismisses the notification
         * if  it is currently displaying an alert.
         * @param localNotification Handle to a local notification object.
         */
        virtual void unregisterLocalNotification(
            LocalNotification* localNotification);

        /**
         * Registers the current application for receiving push notifications.
         * @param types A bit mask specifying the types of notifications the
         * application accepts.
         * See PushNotificationType for valid bit-mask values.
         * This param is applied only on iOS platform. Android platform will
         * ignore this value.
         * @param accountID Is the ID of the account authorized to send messages
         * to the application, typically the email address of an account set up
         * by the application's developer.
         * On iOS platform this param is ignored.
         *
         * Example: Notification::getInstance->registerPushNotification(
         *  PUSH_NOTIFICATION_TYPE_BADGE | PUSH_NOTIFICATION_TYPE_ALERT);
         *
         *  @return One of the next result codes:
         *  - MA_NOTIFICATION_RES_OK if no error occurred.
         *  - MA_NOTIFICATION_ALREADY_REGISTERED if the application is already
         *    registered for receiving push notifications.
         */
        virtual int registerPushNotification(const int types,
            const MAUtil::String& accountID);

        /**
         * Unregister application for push notifications.
         */
        virtual void unregisterPushNotification();

        /**
         * Add listener for push notifications received by this application.
         * @param listener The listener that will receive
         * push notification events.
         * Don't forget to register the application for receiving push
         * notifications by calling registerPushNotification function.
         */
        virtual void addPushNotificationListener(
            PushNotificationListener* listener);

        /**
         * Remove listener for push notifications received by this application.
         * @param listener The listener that receives push notification events.
         */
        virtual void removePushNotificationListener(
            PushNotificationListener* listener);

        /**
         * Set the number currently set as the badge of the application icon.
         * Platform: iOS only.
         * @param iconBadgeNumber The number that will be set as the badge of
         * the application icon.
         * If this value is negative this method will do nothing.
         */
        virtual void setApplicationIconBadgeNumber(const int iconBadgeNumber);

        /**
         * Get the number currently set as the badge of the application icon.
         * Platform: iOS only.
         * @return The number currently set as the badge of the application icon.
         */
        virtual int getApplicationIconBadgeNumber();

    protected:
        /**
         * Constructor is protected since this is a singleton.
         * (subclasses can still create instances).
         */
        NotificationManager();

    private:
        /**
         * The single instance of this class.
         */
        static NotificationManager* sInstance;

        /**
         * Dictionary of local notifications identified by handles.
         */
        MAUtil::Map<MAHandle, LocalNotification*> mLocalNotificationMap;

        /**
         * Array with push notification listeners.
         */
        MAUtil::Vector<PushNotificationListener*> mPushNotificationListeners;
    };

} // namespace Notification

#endif /* NOTIFICATION_NOTIFICATION_MANAGER_H_ */
