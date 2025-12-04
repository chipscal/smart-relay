using System;

namespace Clab.Smart.Relay.App;

public class TopicTreeNode<V>(string topic, V value) : Dictionary<string, TopicTreeNode<V>>
{
    public string   Topic { get; private set; } = topic;
    public V        Value { get; private set; } = value;

    public bool TryTreeAdd(TopicTreeNode<V> child)
    {
        if (child.Topic == Topic)
        {
            throw new InvalidOperationException("Already present");
        }

        if (TopicMatch(child.Topic, Topic)) // if Topic is most generic
        {
            foreach (var subTree in this.Values.ToArray())
            {
                if (TopicMatch(subTree.Topic, child.Topic)) // if child.Topic is more generic than subtree
                {
                    if (this.Remove(subTree.Topic))
                    {
                        if (child.TryTreeAdd(subTree))
                        {
                            this[child.Topic] = child;
                            continue;
                        }
                    }
                }

                if (subTree.TryTreeAdd(child))
                    return true;
            }

            this[child.Topic] = child;
            return true;
        }

        return false;

    }

    public TopicTreeNode<V> TryTreeFind(string topic)
    {
        if (topic == this.Topic)
        {
            return this;
        }

        if (TopicMatch(topic, Topic)) // if this.Topic is most generic
        {
            foreach (var subTree in this.Values.ToArray())
            {
                var found = subTree.TryTreeFind(topic);
                if (found != null)
                    return found;
            }
        }

        return null;
    }

    public bool TryTreeRemove(string topic)
    {
        if (topic == this.Topic)
        {
            throw new InvalidOperationException("Cannot remove myself!");
        }

        if (TopicMatch(topic, Topic)) // if Topic is most generic
        {
            foreach (var subTree in this.Values.ToArray())
            {
                if (subTree.Topic == topic)
                {
                    if (this.Remove(subTree.Topic))
                    {
                        foreach (var child in subTree.Values)
                        {
                            this[child.Topic] = child;
                        }
                        return true;
                    }
                }

                if (subTree.TryTreeRemove(topic))
                    return true;
            }
        }

        return false;
    }

    public IEnumerable<V> GetTreeMatches(string topic)
    {
        
        if (TopicMatch(topic, Topic))
        {
            yield return Value;
            foreach (var subTree in this.Values)
            {
                foreach (var value in subTree.GetTreeMatches(topic))
                    yield return value;
            }
        }
    }

    private static bool TopicMatch(string matchTopic, string testTopic)
    {
        var matchTopicPath = matchTopic.Split("/");
        var testTopicPath = testTopic.Split("/");


        string  matchShare = null;
        int     matchStart = 0;
        if (matchTopicPath[0].ToLowerInvariant() == "$share") //shared subscription support
        {
            matchShare = matchTopicPath[1].ToLowerInvariant();
            matchStart = 2;
        } 

        string  testShare = null;
        int     testStart = 0;
        if (testTopicPath[0].ToLowerInvariant() == "$share") //shared subscription support
        {
            testShare = testTopicPath[1].ToLowerInvariant();
            testStart = 2;
        }

        if (!string.IsNullOrWhiteSpace(matchShare) && !string.IsNullOrWhiteSpace(testShare) && matchShare != testShare)
            return false;

        if (testTopicPath.Last() != "#" && (testTopicPath.Length - testStart) != (matchTopicPath.Length - matchStart))
            return false; 


        bool topicMatched = true;
        for (int k = testStart; k < testTopicPath.Length; k++)
        {
            if (k == testTopicPath.Length - 1 && testTopicPath[k] == "#")
                break;

            if (testTopicPath[k] != "+" && testTopicPath[k].ToUpperInvariant() != matchTopicPath[k - testStart + matchStart].ToUpperInvariant())
            {
                topicMatched = false;
                break;
            }   
        }

        return topicMatched;
    }

    private static string GetMostGenericTopic(string t1, string t2)
    {
        if (TopicMatch(t2, t1))
            return t1;
        else if (TopicMatch(t1, t2))
            return t2;
        else
            return null;
    }
}

public class TopicTree<V>() : TopicTreeNode<V>("#", default)
{
    
}