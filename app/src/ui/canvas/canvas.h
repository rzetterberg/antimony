#ifndef CANVAS_H
#define CANVAS_H

#include <Python.h>

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPointer>
#include <QMenu>

#include "graph/node/constructor.h"

class Node;
class NodeInspector;
class Link;
class GraphScene;
namespace Ui { class MainWindow; }

class Canvas : public QGraphicsView
{
    Q_OBJECT
public:
    explicit Canvas(Graph* graph, QWidget* parent=0);
    explicit Canvas(Graph* graph, GraphScene* scene, QWidget* parent=0);

    void makeNodeAtCursor(NodeConstructorFunction f);

    NodeInspector* getNodeInspector(Node* n) const;

    /*
     *  Adds extra menu items that create input and output datums.
     */
    static void populateDatumCommands(QMenu* menu, GraphNode* g);

    /*
     *  Action called to create a new datum (only used in subgraphs)
     */
    static void makeDatumAction(GraphNode* n, PyTypeObject* t, bool output);

    Graph* const graph;
    GraphScene* scene;

public slots:
    void onCopy();
    void onCut();
    void onPaste();

    void onJumpTo(Node* node);

protected:
    /*
     *  Paste in an array of nodes, properly updating UIDs and names
     */
    void pasteNodes(QJsonArray array);

    /** On mouse press, save click position (for panning).
     */
    void mousePressEvent(QMouseEvent* event) override;

    /* On mouse drag, pan the scene (if left-click).
     */
    void mouseMoveEvent(QMouseEvent* event) override;

    /*
     *  On mouse release, do the multi-select thing.
     */
    void mouseReleaseEvent(QMouseEvent* event) override;

    /*  When the scroll wheel is rolled, zoom about the cursor.
     */
    void wheelEvent(QWheelEvent* event) override;

    /** On delete key press, delete nodes and connections.
     */
    void keyPressEvent(QKeyEvent *event) override;

    /** Draws pixel grid in the background
     */
    void drawBackground(QPainter* painter, const QRectF& rect) override;

    /*
     *  Draws selection area in the foreground.
     */
    void drawForeground(QPainter* painter, const QRectF& rect) override;

    /** Pans the scene rectangle.
     */
    void pan(QVector3D d);

    /*
     *  Properties used to animate zooming to a particular node.
     */
    void setCenter(QPointF p);
    QPointF getCenter() const;
    Q_PROPERTY(QPointF CENTER READ getCenter WRITE setCenter);
    void setZoom(float z);
    float getZoom() const;
    Q_PROPERTY(float ZOOM READ getZoom WRITE setZoom);

    /*
     *  Deletes all selected objects in an undo-able way
     */
    void deleteSelected();

    QPointF click_pos;
    QPointF drag_pos;
    bool selecting;
};

#endif // CANVAS_H
