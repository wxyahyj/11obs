#pragma once

#include "StreamConfig.h"
#include "StreamController.h"

class MainWindow {
public:
    MainWindow();
    ~MainWindow();

    void draw(StreamConfig& config, StreamController& controller);

private:
    void drawConfigPanel(StreamConfig& config);
    void drawControlPanel(StreamConfig& config, StreamController& controller);
    void drawStatsPanel(StreamController& controller);

private:
    bool showConfig = true;
    bool showStats = true;
};
