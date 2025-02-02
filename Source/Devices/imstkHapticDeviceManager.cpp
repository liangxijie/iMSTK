/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include "imstkHapticDeviceManager.h"
#include "imstkHapticDeviceClient.h"
#include "imstkLogger.h"

#include <HD/hd.h>

namespace imstk
{
std::shared_ptr<HapticDeviceClient>
HapticDeviceManager::makeDeviceClient(std::string name)
{
    auto deviceClient = std::shared_ptr<HapticDeviceClient>(new HapticDeviceClient(name));
    m_deviceClients.push_back(deviceClient);
    return deviceClient;
}

bool
HapticDeviceManager::initModule()
{
    for (const auto& client : m_deviceClients)
    {
        client->initialize();
    }
    hdStartScheduler();
    return true;
}

void
HapticDeviceManager::uninitModule()
{
    // \todo: Other threads could be mid update call here
    hdStopScheduler();
    for (const auto& client : m_deviceClients)
    {
        client->disable();
    }
}
} // imstk
