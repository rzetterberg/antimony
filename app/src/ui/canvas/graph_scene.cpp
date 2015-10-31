#include <Python.h>

#include "ui/canvas/graph_scene.h"
#include "ui/canvas/canvas.h"
#include "ui/canvas/connection.h"
#include "ui/canvas/inspector/inspector.h"
#include "ui/canvas/port.h"
#include "ui/util/scene.h"

#include "app/app.h"
#include "app/undo/undo_move.h"

#include "graph/hooks/hooks.h"

#include "graph/datum.h"
#include "graph/graph.h"
#include "graph/graph_node.h"

GraphScene::GraphScene(Graph* graph, QObject* parent)
    : QGraphicsScene(parent), graph(graph)
{
    graph->installWatcher(this);
    graph->installExternalHooks(new AppHooks(this));
    connect(this, &GraphScene::jumpTo,
            App::instance(), &App::jumpToInViewport);
}

Canvas* GraphScene::newCanvas()
{
    return new Canvas(graph, this);
}

void GraphScene::trigger(const GraphState& state)
{
    // Store nodes in a Set for O(1) checking
    QSet<Node*> nodes;
    for (auto n : state.nodes)
        nodes.insert(n);

    QSet<Datum*> datums;
    for (auto d : state.incoming)
        datums.insert(d);

    pruneHash(nodes, &inspectors);
    pruneHash(nodes, &subgraphs);

    pruneHash(datums, &inputs);
    pruneHash(datums, &outputs);

    for (auto n : nodes)
    {
        if (!inspectors.contains(n))
            makeUIfor(n);
        if (auto gn = dynamic_cast<GraphNode*>(n))
            if (!subgraphs.contains(gn))
                subgraphs[gn].reset(new GraphScene(gn->getGraph()),
                                    &QObject::deleteLater);
    }

    for (auto d : datums)
    {
        Port* p = NULL;
        if (d->isOutput() && !outputs.contains(d))
        {
            outputs[d].reset(new InputPort(d, NULL));
            p = outputs[d].data();
        }
        else if (!d->isOutput() && !inputs.contains(d))
        {
            inputs[d].reset(new OutputPort(d, NULL));
            p = inputs[d].data();
        }

        if (p)
        {
            addItem(p);
        }
    }
}

void GraphScene::onGlowChange(Node* n, bool g)
{
    getInspector(n)->setGlow(g);
}

void GraphScene::makeUIfor(Node* n)
{
    auto i = new NodeInspector(n);
    inspectors[n].reset(i, &QObject::deleteLater);
    addItem(i);

    if (views().length() > 0)
    {
        Q_ASSERT(dynamic_cast<Canvas*>(views().back()));
        auto c = static_cast<Canvas*>(views().back());
        i->setPos(c->mapToScene(c->geometry().center()) -
                  QPointF(i->boundingRect().width()/2,
                          i->boundingRect().height()/2));
    }

    // If we've cached a title (e.g. because a Node called title
    // before its inspector was created), assign it here.
    if (title_cache.contains(n))
    {
        i->setTitle(title_cache[n]);
        title_cache.remove(n);
    }
    if (export_cache.contains(n))
    {
        i->setExportWorker(export_cache[n]);
        export_cache.remove(n);
    }

    connect(i, &NodeInspector::glowChanged,
            this, &GraphScene::onGlowChange);
    connect(i, &NodeInspector::glowChanged,
            this, &GraphScene::glowChanged);
}

Connection* GraphScene::makeLinkFrom(Datum* d)
{
    Connection* c;
    if (inspectors.contains(d->parentNode()))
        c = new Connection(inspectors[d->parentNode()]->outputPort(d));
    else if (inputs.contains(d))
        c = new Connection(inputs[d].data());
    else
        Q_ASSERT(false);
    addItem(c);
    return c;
}

void GraphScene::makeLink(const Datum* source, InputPort* target)
{
    Q_ASSERT(inspectors.contains(source->parentNode()));
    inspectors[source->parentNode()]->makeLink(source, target);
}

NodeInspector* GraphScene::getInspector(Node* node) const
{
    Q_ASSERT(inspectors.contains(node));
    return inspectors[node].data();
}

template <class T>
T* GraphScene::getItemAt(QPointF pos) const
{
    for (auto i : items(pos))
        if (auto p = dynamic_cast<T*>(i))
            return p;
    return NULL;
}

void GraphScene::setTitle(Node* node, QString title)
{
    if (inspectors.contains(node))
        inspectors[node]->setTitle(title);
    else
        title_cache[node] = title;
}

void GraphScene::clearExportWorker(Node* node)
{
    if (inspectors.contains(node))
        inspectors[node]->clearExportWorker();
    else if (export_cache.contains(node))
        export_cache.remove(node);
}

void GraphScene::setExportWorker(Node* node, ExportWorker* worker)
{
    if (inspectors.contains(node))
        inspectors[node]->setExportWorker(worker);
    else
        export_cache[node] = worker;
}

NodeInspector* GraphScene::getInspectorAt(QPointF pos) const
{
    return getItemAt<NodeInspector>(pos);
}

InputPort* GraphScene::getInputPortAt(QPointF pos)
{
    return getItemAt<InputPort>(pos);
    for (auto i : items(pos))
        if (auto p = dynamic_cast<InputPort*>(i))
            return p;
    return NULL;
}

InputPort* GraphScene::getInputPortNear(QPointF pos, Datum* d)
{
    float distance = INFINITY;
    InputPort* port = NULL;

    for (auto i : items())
    {
        InputPort* p = dynamic_cast<InputPort*>(i);
        if (p && p->isVisible() && (d == NULL ||
                                    p->getDatum()->acceptsLink(d)))
        {
            QPointF delta = p->mapToScene(p->boundingRect().center()) - pos;
            float d = QPointF::dotProduct(delta, delta);
            if (d < distance)
            {
                distance = d;
                port = p;
            }
        }
    }

    return port;
}

void GraphScene::raiseInspector(NodeInspector* i)
{
    removeItem(i);
    addItem(i);
}

void GraphScene::raiseInspectorAt(QPointF pos)
{
    auto i = getInspectorAt(pos);
    if (i)
        raiseInspector(i);
}

QMap<Node*, QPointF> GraphScene::inspectorPositions() const
{
    QMap<Node*, QPointF> out;

    for (auto m : items())
        if (auto i = dynamic_cast<NodeInspector*>(m))
            out[i->getNode()] = i->pos();

    return out;
}

void GraphScene::setInspectorPositions(QMap<Node*, QPointF> p)
{
    for (auto m : items())
        if (auto i = dynamic_cast<NodeInspector*>(m))
            if (p.contains(i->getNode()))
                i->setPos(p[i->getNode()]);

    for (auto sub : subgraphs)
        sub->setInspectorPositions(p);
}

void GraphScene::endDrag(QPointF delta)
{
    App::instance()->beginUndoMacro("'drag'");
    for (auto m : selectedItems())
        if (auto i = dynamic_cast<NodeInspector*>(m))
            App::instance()->pushStack(new UndoMoveCommand(
                        this, i->getNode(), i->pos() - delta, i->pos()));
    App::instance()->endUndoMacro();
}

GraphScene* GraphScene::getSubscene(const GraphNode* n) const
{
    return subgraphs[const_cast<GraphNode*>(n)].data();
}
