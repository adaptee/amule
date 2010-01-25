//
// This file is part of the aMule Project.
//
// Copyright (c) 2010 Werner Mahr (Vollstrecker) <amule@vollstreckernet.de>
//
// Any parts of this program contributed by third-party developers are copyrighted
// by their respective authors.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301, USA
//

#include "plasma-applet-plasmamule.h"

#include <kdebug.h>

#include <plasma/svg.h>
#include <plasma/theme.h>
#include <plasma/version.h>

#include <QFontMetrics>
#include <QPainter>
#include <QSizeF>
#include <QStringList>
#include <QTime>

K_EXPORT_PLASMA_APPLET(plasma-applet-plasmamule, PlasmaMuleApplet)

PlasmaMuleApplet::PlasmaMuleApplet(QObject *parent, const QVariantList &args)
	: Plasma::Applet(parent, args),
	m_svg(this)
{
	QString path = __IMG_PATH__;
	path.append( "amule_applet.svg");
	m_svg.setImagePath(path);
	setBackgroundHints(TranslucentBackground);
	setMinimumSize(200, 200);
	setMaximumSize(300, 300);

}

PlasmaMuleApplet::~PlasmaMuleApplet()
{
}
 
void PlasmaMuleApplet::init()
{
	connectToEngine();
}

void PlasmaMuleApplet::paintInterface(QPainter *painter,
	const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
	painter->setRenderHint(QPainter::SmoothPixmapTransform);
	painter->setRenderHint(QPainter::Antialiasing);
	QPixmap pixmap;
	pixmap = m_svg.pixmap();
	QPixmap temp(pixmap.size());
	temp.fill(Qt::transparent);
	QPainter p(&temp);
	p.setCompositionMode(QPainter::CompositionMode_Source);
	p.drawPixmap(0, 0, pixmap);
	p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
	p.fillRect(temp.rect(), QColor(0, 0, 0, 200));
	p.end();
	pixmap = temp;
	painter->drawPixmap(contentsRect, pixmap);
	painter->save();
	painter->setPen(Qt::red);
	QFont font = painter->font();
	font.setBold(true);
	painter->setFont(font);
	if (!m_config_found)
	{	
		painter->drawText(contentsRect, Qt::AlignCenter,
		"aMule not configured or installed");
	}
	else if (!m_os_active)
	{	
		painter->drawText(contentsRect, Qt::AlignCenter,
		"Online Signature disabled");
	} else if (m_uptime == 0)
	{
		painter->drawText(contentsRect, Qt::AlignCenter,
		"aMule is not running\n");
	} else {
		QString message;
		painter->setPen(Qt::black);
		int time, days, hours, minutes;
		QString runtime;
		days = m_uptime/86400;
		time = m_uptime%86400;
		hours=time/3600;
		time=time%3600;
		minutes=time/60;
		time=time%60;

		runtime = QTime (hours, minutes, time).toString(Qt::ISODate);
		if (days)
		{
			if (days = 1)
			{
				runtime.prepend(QString("%1 day ").arg(days));
			} else {
				runtime.prepend(QString("%1 days ").arg(days));
			}
		}
		message = QString("%1 has been running for %2 \n\n").arg(m_version).arg(runtime);
		if (m_ed2k_state == 0 && m_kad_status == 0)
		{
			message.append(QString("%1 is not connected ").arg(m_nickname));
			message.append("but running\n\n");
		}
		else if (m_ed2k_state == 0 && m_kad_status != 0)
		{
			message.append(QString("%1 is connected to ").arg(m_nickname));
			if (m_kad_status == 1)
			{
				message.append("Kad: firewalled \n\n");
			} else {
				message.append("Kad: ok \n\n");
			}
		} else {
			message.append(QString("%1 is connected to %2 [%3:%4] with ").arg(m_nickname).arg(m_ed2k_server_name).arg(m_ed2k_server_ip).arg(m_ed2k_server_port));
			if (m_kad_status == 1)
			{
				if (m_ed2k_id_high_low == "H")
				{
					message.append("HighID | Kad: firewalled \n\n");
				} else {
					message.append("LowID | Kad: firewalled \n\n");
				}
			} else if (m_kad_status == 2)
			{
				if (m_ed2k_id_high_low == "H")
				{
					message.append("HighID | Kad: ok \n\n");
				} else {
					message.append("LowID | Kad: ok \n\n");
				}
			} else {
				if (m_ed2k_id_high_low == "H")
				{
					message.append("HighID | Kad: off \n\n");
				} else {
					message.append("LowID | Kad: off \n\n");
				}
			}
		}
		message.append(QString("Total Download: %1, Upload: %2\n\n").arg(calcSize(m_total_bytes_downloaded)).arg(calcSize(m_total_bytes_uploaded)));
		message.append(QString("Session Download: %1, Upload: %2\n\n").arg(calcSize(m_session_bytes_downloaded)).arg(calcSize(m_session_bytes_uploaded)));
		message.append(QString("Download: %L1 kB/s, Upload: %L2 kB/s\n\n").arg(m_down_speed, 0 , 'f', 1).arg(m_up_speed,0 ,'f', 1));
		QString files_unit;
		if (m_shared_files_count == 1)
		{
			files_unit = "file";
		} else {
			files_unit = "files";
		}
		message.append(QString("Sharing: %1 %2, Clients on queue: %3").arg(m_shared_files_count).arg(files_unit).arg(m_clients_in_up_queue));
		painter->drawText(contentsRect, Qt::TextDontClip | Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap, message);

		if (painter->boundingRect(contentsRect, Qt::TextDontClip | Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap,
			 message).height() > contentsRect.height())
		{
			kDebug() << "Resizing";
			resize(painter->boundingRect(contentsRect, message).height()+(contentsRect.topLeft().y()*2),
				painter->boundingRect(contentsRect, message).height()+(contentsRect.topLeft().y()*2));
		}
	}
	painter->restore();
}

QString PlasmaMuleApplet::calcSize (qlonglong in_size)
{
	int unit=0;
	double size;
	QStringList units;
	units << "Bytes" << "KBs" << "MBs" << "GBs" << "TBs" << "PBs" << "BBs" << "ZBs" << "YBs";

	while (in_size >1023)
	{
		in_size /= 1024;
		unit++;
	}

	size = (in_size * 1024) / 1024;
	return QString("%L1 %2").arg(size, 0, 'f', 2).arg(units.at(unit));
}

void PlasmaMuleApplet::connectToEngine()
{
	m_aMuleEngine = dataEngine("plasmamule");
	m_aMuleEngine->connectAllSources(this, 0);
	connect(m_aMuleEngine, SIGNAL(sourceAdded(const QString&)), this, SLOT(onSourceAdded(const QString&)));
	connect(m_aMuleEngine, SIGNAL(sourceRemoved(const QString&)), this, SLOT(onSourceRemoved(const QString&)));
}

void PlasmaMuleApplet::onSourceAdded(const QString& source)
{
	kDebug() << "New Source: " << source << " added";
	m_aMuleEngine->connectSource(source, this, 0);
}

void PlasmaMuleApplet::onSourceRemoved(const QString& source)
{
	kDebug() << "Source: " << source << " removed";
	update();
}

void PlasmaMuleApplet::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
	bool needs_update = FALSE;
	kDebug() << "Updating data";
	if (data["os_active"].toBool() != m_os_active && data.contains("os_active"))
	{
		m_os_active = data["os_active"].toBool();
		needs_update = TRUE;
	}
	if (data["config_found"].toBool() != m_config_found && data.contains("config_found"))
	{
		m_config_found = data["config_found"].toBool();
		needs_update = TRUE;
	}
	if (data["ed2k_state"].toInt() != m_ed2k_state && data.contains("ed2k_state"))
	{
		m_ed2k_state = data["ed2k_state"].toInt();
		needs_update = TRUE;
	}
	if (data["ed2k_server_name"] != m_ed2k_server_name && data.contains("ed2k_server_name"))
	{
		m_ed2k_server_name = data["ed2k_server_name"].toString();
		needs_update = TRUE;
	}
	if (data["ed2k_server_ip"] != m_ed2k_server_ip && data.contains("ed2k_server_ip"))
	{
		m_ed2k_server_ip = data["ed2k_server_ip"].toString();
		needs_update = TRUE;
	}
	if (data["ed2k_server_port"].toInt() != m_ed2k_server_port && data.contains("ed2k_server_port"))
	{
		m_ed2k_server_port = data["ed2k_server_port"].toInt();
		needs_update = TRUE;
	}
	if (data["ed2k_id_high_low"] != m_ed2k_id_high_low && data.contains("ed2k_id_high_low"))
	{
		m_ed2k_id_high_low = data["ed2k_id_high_low"].toString();
		needs_update = TRUE;
	}
	if (data["kad_status"].toInt() != m_kad_status && data.contains("kad_status"))
	{
		m_kad_status = data["kad_status"].toInt();
		needs_update = TRUE;
	}
	if (data["down_speed"].toInt() != m_down_speed && data.contains("down_speed"))
	{
		m_down_speed = data["down_speed"].toDouble();
		needs_update = TRUE;
	}
	if (data["up_speed"].toDouble() != m_up_speed && data.contains("up_speed"))
	{
		m_up_speed = data["up_speed"].toDouble();
		needs_update = TRUE;
	}
	if (data["clients_in_up_queue"].toInt() != m_clients_in_up_queue && data.contains("clients_in_up_queue"))
	{
		m_clients_in_up_queue = data["clients_in_up_queue"].toInt();
		needs_update = TRUE;
	}
	if (data["shared_files_count"].toInt() != m_shared_files_count && data.contains("shared_files_count"))
	{
		m_shared_files_count = data["shared_files_count"].toInt();
		needs_update = TRUE;
	}
	if (data["nickname"] != m_nickname && data.contains("nickname"))
	{
		m_nickname = data["nickname"].toString();
		needs_update = TRUE;
	}
	if (data["total_bytes_downloaded"].toLongLong() != m_total_bytes_downloaded && data.contains("total_bytes_downloaded"))
	{
		m_total_bytes_downloaded = data["total_bytes_downloaded"].toLongLong();
		needs_update = TRUE;
	}
	if (data["total_bytes_uploaded"].toLongLong() != m_total_bytes_uploaded && data.contains("total_bytes_uploaded"))
	{
		m_total_bytes_uploaded = data["total_bytes_uploaded"].toLongLong();
		needs_update = TRUE;
	}
	if (data["version"] != m_version && data.contains("version"))
	{
		m_version = data["version"].toString();
		needs_update = TRUE;
	}
	if (data["session_bytes_downloaded"].toLongLong() != m_session_bytes_downloaded && data.contains("session_bytes_downloaded"))
	{
		m_session_bytes_downloaded = data["session_bytes_downloaded"].toLongLong();
		needs_update = TRUE;
	}
	if (data["session_bytes_uploaded"].toLongLong() != m_session_bytes_uploaded && data.contains("session_bytes_uploaded"))
	{
		m_session_bytes_uploaded = data["session_bytes_uploaded"].toLongLong();
		needs_update = TRUE;
	}
	if (data["uptime"].toInt() != m_uptime && data.contains("uptime"))
	{
		m_uptime = data["uptime"].toInt();
		needs_update = TRUE;
	}

	if (needs_update)
	{
		kDebug() << "Updating view";
		update();
	}
}

#include "plasma-applet-plasmamule.moc"
