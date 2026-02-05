using System;
using System.Collections.Generic;
using System.Linq;

namespace Clab.Smart.Relay.App;


public class Utils
{
    /// <summary>
    /// Converts a string (e.g., full name) into uppercase initials.
    /// Handles multiple spaces, punctuation, and empty input.
    /// </summary>
    public static string GetInitials(string input)
    {
        if (string.IsNullOrWhiteSpace(input))
            return string.Empty;

        // Split by spaces, remove empty entries, and take the first character of each word
        var initials = input
            .Trim()
            .Split(new char[] { ' ', '\t', '\n' }, StringSplitOptions.RemoveEmptyEntries)
            .Where(word => char.IsLetterOrDigit(word[0])) // Ignore non-alphanumeric starting chars
            .Select(word => char.ToUpper(word[0]))
            .ToArray();

        return new string(initials);
    }

    public static bool TryParseCharEnum<T>(char c, out T value) where T : struct, Enum
    {
        var intValue = (int)c;

        if (!Enum.IsDefined(typeof(T), intValue))
        {
            value = default;
            return false;
        }

        value = (T)Enum.ToObject(typeof(T), intValue);
        return true;
    }

    public static bool TryParseCharEnum<T>(string c, out T value) where T : struct, Enum
    {
        if (string.IsNullOrWhiteSpace(c) || c.Length > 1)
        {
            value = default;
            return false;
        }

        return TryParseCharEnum<T>(c[0], out value);
    }

    public static char ToChar<T>(T value) where T : Enum
    {
        return (char)Convert.ToInt32(value);
    }
}