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
}