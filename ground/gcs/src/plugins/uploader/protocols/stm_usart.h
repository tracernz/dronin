/**
 ******************************************************************************
 * @file       stm_usart.h
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup Uploader Uploader Plugin
 * @{
 * @brief STM32 USART bootloader protocol support
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Additional note on redistribution: The copyright and license notices above
 * must be maintained in each individual source file that is a derivative work
 * of this source file; otherwise redistribution is prohibited.
 */

#ifndef STM_USART_H
#define STM_USART_H

#include <QByteArray>

class stm_usart_cmd {
public:
    enum cmd {
        CMD_GET             = 0x00,
        CMD_GETVER          = 0x01,
        CMD_GETID           = 0x02,
        CMD_READ            = 0x11,
        CMD_GO              = 0x21,
        CMD_WRITE           = 0x31,
        CMD_ERASE           = 0x43,
        CMD_EXTERASE        = 0x44,
        CMD_WRITE_PROTECT   = 0x63,
        CMD_WRITE_UNPROTECT = 0x73,
        CMD_READ_PROTECT    = 0x82,
        CMD_READ_UNPROTECT  = 0x92,
    };

    stm_usart_cmd(enum cmd cmd);

    void setPayload(QByteArray payload);
    bool send(/* TODO: IO stream? */);

private:
    enum cmd m_cmd;
    QByteArray m_payload;
    quint8 m_crc;
};

class stm_usart
{
public:
    stm_usart();
private:

};

#endif // STM_USART_H

/**
 * @}
 * @}
 */
