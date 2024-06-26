/*
 * Copyright (c) 2015, Intel Corporation
 *   Author: Xavier Hallade <xavier.hallade@intel.com>
 * Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
 
#pragma once

#include "api/plugin-api.hpp"
#include "ui_ftnoir_tracker_rs_controls.h"
#include  "ftnoir_tracker_rs_worker.h"
#include <QTimer>

class ImageWidget;

class RSTracker : public QObject, public ITracker
{
    Q_OBJECT

public:
    RSTracker();
    ~RSTracker() override;
    module_status start_tracker(QFrame *) override;
    void data(double *data) override;

protected:
    void configurePreviewFrame();

private:
    RSTrackerWorkerThread mTrackerWorkerThread;
    QTimer mPreviewUpdateTimer;
    QWidget *mPreviewFrame;
    ImageWidget *mImageWidget = nullptr;
    const int kPreviewUpdateInterval = 30;

private slots:
    void showRealSenseErrorMessageBox(int exitCode);
    void startPreview();
    void updatePreview();
    void stopPreview();
    void handleTrackingEnded(int exitCode);

};

class RSTrackerMetaData : public Metadata
{
    Q_OBJECT

    QString name() override;
    QIcon icon() override;
};
