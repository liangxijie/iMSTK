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

#include "imstkTexture.h"

namespace imstk
{
Texture::Texture(std::string path, Type type)
{
    m_path = path;
    m_type = type;
}

Texture::Type
Texture::getType() const
{
    return m_type;
}

const std::string
Texture::getPath() const
{
    return m_path;
}

const Texture::FileType
Texture::getFileType()
{
    FileType textureType = FileType::UNKNOWN;

    std::string extString = m_path.substr(m_path.find_last_of(".") + 1);

    if (extString.empty() || m_path.find(".") == std::string::npos)
    {
        return textureType;
    }

    if (extString == "bmp" || extString == "BMP")
    {
        textureType = FileType::BMP;
    }
    else if (extString == "png" || extString == "PNG")
    {
        textureType = FileType::PNG;
    }
    else if (extString == "jpg" || extString == "JPG"
             || extString == "jpeg" || extString == "JPEG")
    {
        textureType = FileType::JPG;
    }
    else if (extString == "dds" || extString == "DDS")
    {
        textureType = FileType::DDS;
    }

    return textureType;
}

bool
Texture::getMipmapsEnabled()
{
    return m_mipmapsEnabled;
}

bool
Texture::isAnisotropyEnabled()
{
    return m_anisotropyEnabled;
}

float
Texture::getAnisotropyFactor()
{
    return m_anisotropyFactor;
}
}