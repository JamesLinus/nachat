#ifndef NATIVE_CHAT_MATRIX_SESSION_H_
#define NATIVE_CHAT_MATRIX_SESSION_H_

#include <unordered_map>
#include <chrono>

#include <QtNetwork>
#include <QObject>
#include <QString>
#include <QUrlQuery>

#include "../QStringHash.hpp"

#include "Room.hpp"
#include "Content.hpp"

namespace matrix {

namespace proto {
struct Sync;
}

class Matrix;

class ContentFetch : public QObject {
  Q_OBJECT

public:
  ContentFetch(QObject *parent = nullptr) : QObject(parent) {}

signals:
  void finished(const Content &content, const QString &type, const QString &disposition, const QByteArray &data);
  void error(const QString &msg);
};

enum class ThumbnailMethod { CROP, SCALE };

class Session : public QObject {
  Q_OBJECT

public:
  Session(Matrix& universe, QUrl homeserver, QString user_id, QString access_token);

  Session(const Session &) = delete;
  Session &operator=(const Session &) = delete;

  const QString &access_token() const { return access_token_; }
  const QString &user_id() const { return user_id_; }

  void log_out();

  bool synced() const { return synced_; }
  std::vector<Room *> rooms();

  size_t buffer_size() const { return buffer_size_; }
  void set_buffer_size(size_t size) { buffer_size_ = size; }

  QNetworkReply *get(const QString &path, QUrlQuery query = QUrlQuery());

  QNetworkReply *post(const QString &path, QJsonObject body = QJsonObject(), QUrlQuery query = QUrlQuery());
  QNetworkReply *post(const QString &path, QIODevice *data, QUrlQuery query = QUrlQuery());

  QNetworkReply *put(const QString &path, QJsonObject body);

  ContentFetch *get(const Content &);

  ContentFetch *get_thumbnail(const Content &, const QSize &size, ThumbnailMethod method = ThumbnailMethod::SCALE);

signals:
  void logged_out();
  void error(QString message);
  void synced_changed();
  void joined(matrix::Room &room);
  void sync_progress(qint64 received, qint64 total);
  void sync_complete();

private:
  Matrix &universe_;
  const QUrl homeserver_;
  const QString user_id_;
  size_t buffer_size_;
  QString access_token_;
  std::unordered_map<QString, Room, QStringHash> rooms_;
  bool synced_;
  QString next_batch_;

  std::chrono::steady_clock::time_point last_sync_error_;
  // Last time a sync failed. Used to ensure we don't spin if errors happen quickly.

  QNetworkRequest request(const QString &path, QUrlQuery query = QUrlQuery());

  void sync(QUrlQuery query);
  void handle_sync_reply(QNetworkReply *);
  void dispatch(proto::Sync sync);
};

}

#endif
