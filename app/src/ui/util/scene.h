template <class N, class T>
void pruneHash(const QSet<Node*>& nodes, QHash<N*, T>* hash)
{
    auto itr = hash->begin();
    while (itr != hash->end())
        if (!nodes.contains(itr.key()))
            itr = hash->erase(itr);
        else
            itr++;
}
