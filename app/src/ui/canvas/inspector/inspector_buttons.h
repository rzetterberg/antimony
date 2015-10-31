#ifndef INSPECTOR_BUTTONS_H
#define INSPECTOR_BUTTONS_H

#include <QPointer>

#include "ui/util/button.h"

#include "graph/watchers.h"

class NodeInspector;
class ExportWorker;

class ScriptNode;
class GraphNode;

////////////////////////////////////////////////////////////////////////////////

class InspectorScriptButton : public GraphicsButton, ScriptWatcher
{
    Q_OBJECT
public:
    InspectorScriptButton(ScriptNode* n, QGraphicsItem* parent);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget=0) override;

    void trigger(const ScriptState& state) override;
protected slots:
    void onPressed();
protected:
    bool script_valid;
    ScriptNode* node;
};

////////////////////////////////////////////////////////////////////////////////

class InspectorGraphButton : public GraphicsButton
{
    Q_OBJECT
public:
    InspectorGraphButton(GraphNode* n, QGraphicsItem* parent);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget=0) override;
protected slots:
    void onPressed();
protected:
    GraphNode* node;
};

class InspectorViewButton : public GraphicsButton
{
    Q_OBJECT
public:
    InspectorViewButton(GraphNode* n, QGraphicsItem* parent);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget=0) override;
protected slots:
    void onPressed();
protected:
    GraphNode* node;
};

class InspectorQuadButton : public GraphicsButton
{
    Q_OBJECT
public:
    InspectorQuadButton(GraphNode* n, QGraphicsItem* parent);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget=0) override;
protected slots:
    void onPressed();
protected:
    GraphNode* node;
};

////////////////////////////////////////////////////////////////////////////////

class InspectorShowHiddenButton : public GraphicsButton, NodeWatcher
{
    Q_OBJECT
public:
    InspectorShowHiddenButton(QGraphicsItem* parent,
                              NodeInspector* inspector);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget=0) override;
    void trigger(const NodeState& state) override;
protected slots:
    void onPressed();
protected:
    bool toggled;
    NodeInspector* inspector;
};

#endif
