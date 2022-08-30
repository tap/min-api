/// @file
///	@ingroup 	minapi
///	@copyright	Copyright 2022 The Min-API Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#pragma once

namespace c74::min {


    class watcher {
    public:

        /// Create a file watcher.
        /// @param	an_owner	The owning object for the file watcher. Typically you will pass `this`.
        /// @param	a_function	An optional function to be executed when the file watcher issues notifications.
        ///						Typically the function is defined using a C++ lambda with the #MIN_FUNCTION signature.
        /// @param	create_messages	Optionally have min create a "filechanged" message on `an_owner` and
        ///                         associate that with this file watcher instance.
        ///                         *NOTE* if your owner uses the notify method or has multiple buffers, you'll need to set 
        ///                         this to false and set up your own notify method handling.
        /// @see    buffer_reference

        watcher(object_base* an_owner, const function& a_function = nullptr, const bool create_messages = true)
        : m_owner { *an_owner }
        , m_notification_callback { a_function }
        {
            if (create_messages) {
                // Messages added to the owning object for this file watcher instance
                m_message = std::make_unique<message<>>(&m_owner, "filechanged",
                     MIN_FUNCTION {
                        //string filename = args[0];
                        //int path = args[1];
                        atoms a;
                        return m_notification_callback(a,0);
                     }
                );
            }
        }


        // copy/move constructors and assignment
        watcher(const watcher& source) = delete;
        watcher(watcher&& source)      = delete;
        watcher& operator=(const watcher& source) = delete;
        watcher& operator=(watcher&& source) = delete;


        /// Destroy a file watcher.

        ~watcher() {
            end();
        }


        void begin(const path& p) {
            end();

            short path = p.id();
            const char *filename = p.filename();

            m_instance = max::filewatcher_new(m_owner, path, filename);
            max::filewatcher_start(m_instance);
        }


        void end() {
            max::object_free(m_instance);
            m_instance = nullptr;
        }

    private:
        object_base&            m_owner;
        void*                   m_instance {nullptr};
        function                m_notification_callback;
        unique_ptr<message<>>   m_message {};// Message added to owning object for this watcher


    };

    
}    // namespace c74::min
