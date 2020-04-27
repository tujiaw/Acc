#ifndef QPID_MESSAGING_DURATION_H
#define QPID_MESSAGING_DURATION_H

/*
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 */

#include "qpid/messaging/ImportExport.h"

#include "qpid/sys/IntegerTypes.h"

namespace qpid {
namespace messaging {

/**   \ingroup messaging 
 * A duration is a time in milliseconds.
 */
class QPID_MESSAGING_CLASS_EXTERN Duration
{
  public:
    QPID_MESSAGING_EXTERN explicit Duration(uint64_t milliseconds);
    QPID_MESSAGING_EXTERN uint64_t getMilliseconds() const;
    QPID_MESSAGING_EXTERN static const Duration FOREVER;
    QPID_MESSAGING_EXTERN static const Duration IMMEDIATE;
    QPID_MESSAGING_EXTERN static const Duration SECOND;
    QPID_MESSAGING_EXTERN static const Duration MINUTE;
  private:
    uint64_t milliseconds;
};

QPID_MESSAGING_EXTERN Duration operator*(const Duration& duration,
                                         uint64_t multiplier);
QPID_MESSAGING_EXTERN Duration operator*(uint64_t multiplier,
                                         const Duration& duration);
QPID_MESSAGING_EXTERN bool operator==(const Duration& a, const Duration& b);
QPID_MESSAGING_EXTERN bool operator!=(const Duration& a, const Duration& b);

}} // namespace qpid::messaging

#endif  /*!QPID_MESSAGING_DURATION_H*/
