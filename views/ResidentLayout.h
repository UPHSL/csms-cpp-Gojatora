#pragma once

#include <drogon/HttpViewData.h>

#include <string>

// Small helpers shared by the Resident and Service Request views.
// Presentation only: nothing here validates, queries, or changes data.

// HTML-escapes any user-supplied text before it is written into a page.
inline std::string h(const std::string &text)
{
    return drogon::HttpViewData::htmlTranslate(text);
}

inline const char *residentPageStyles()
{
    return R"CSS(
        * { box-sizing: border-box; }
        body { margin: 0; background: #f4f5f7; color: #111; font-family: Arial, Helvetica, sans-serif; }
        main { width: min(1100px, 94%); margin: 24px auto; background: #fff; padding: 28px;
               border: 1px solid #d9dde3; border-radius: 12px; }
        h1 { margin: 0 0 16px; }
        a { color: #1a56b0; }
        .toolbar { display: flex; flex-wrap: wrap; gap: 12px; align-items: end; justify-content: space-between;
                   margin-bottom: 20px; }
        .search { display: flex; flex-wrap: wrap; gap: 8px; align-items: end; }
        label { display: block; font-weight: 700; margin-bottom: 4px; }
        input[type=text], input[type=search], input[type=email], input[type=tel] {
            width: 100%; padding: 8px; border: 1px solid #9aa3af; border-radius: 6px; font-size: 1rem; }
        .search input { width: 260px; }
        .button, button { display: inline-block; padding: 8px 14px; border: 1px solid #1a56b0; border-radius: 6px;
                          background: #1a56b0; color: #fff; font-size: 1rem; text-decoration: none; cursor: pointer; }
        .button.secondary, button.secondary { background: #fff; color: #1a56b0; }
        .table-wrap { overflow-x: auto; }
        table { width: 100%; border-collapse: collapse; }
        th, td { text-align: left; padding: 8px 10px; border-bottom: 1px solid #e1e4e8; vertical-align: middle; }
        th { background: #f0f2f5; }
        .badge { display: inline-block; padding: 2px 10px; border-radius: 999px; font-size: 0.9rem; font-weight: 700; }
        .badge.active { background: #dff5e1; color: #17612b; }
        .badge.inactive { background: #e8e8e8; color: #555; }
        .badge.pending { background: #fff3cd; color: #7a5a00; }
        .badge.inprogress { background: #dbe8fb; color: #174a99; }
        .badge.completed { background: #dff5e1; color: #17612b; }
        .badge.cancelled { background: #e8e8e8; color: #555; }
        .site-nav { background: #1a56b0; padding: 0 3%; display: flex; gap: 4px; }
        .site-nav a { color: #fff; text-decoration: none; padding: 12px 16px; font-weight: 700; }
        .site-nav a.current { background: rgba(255, 255, 255, 0.2); }
        select, textarea, input[type=date] {
            width: 100%; padding: 8px; border: 1px solid #9aa3af; border-radius: 6px; font-size: 1rem;
            font-family: inherit; background: #fff; }
        textarea { min-height: 120px; resize: vertical; }
        .search select { width: 180px; }
        .details { display: grid; grid-template-columns: max-content 1fr; gap: 8px 24px; margin: 0 0 20px; }
        .details dt { font-weight: 700; }
        .details dd { margin: 0; white-space: pre-wrap; overflow-wrap: anywhere; }
        .terminal-note { color: #555; font-style: italic; }
        .actions { display: flex; gap: 8px; align-items: center; }
        .actions form { margin: 0; }
        .message { padding: 10px 14px; border-radius: 6px; margin-bottom: 16px; }
        .message.success { background: #dff5e1; color: #17612b; border: 1px solid #a9dbb0; }
        .message.error { background: #fde8e8; color: #8a1c1c; border: 1px solid #f1b4b4; }
        .message.error ul { margin: 6px 0 0 18px; padding: 0; }
        .field { margin-bottom: 16px; }
        .field-error { color: #8a1c1c; margin-top: 4px; font-size: 0.95rem; }
        .readonly { margin-bottom: 16px; }
        .readonly span { font-weight: 700; }
        .form-actions { display: flex; gap: 10px; margin-top: 20px; }
        .empty { padding: 16px 0; color: #444; }
    )CSS";
}

// The shared top navigation. `current` is "residents" or "service-requests".
inline std::string navigationBar(const std::string &current)
{
    auto link = [&](const char *key, const char *href, const char *label) {
        return std::string("<a href=\"") + href + "\"" +
               (current == key ? " class=\"current\"" : "") + ">" + label + "</a>";
    };
    return "<nav class=\"site-nav\" aria-label=\"Main\">" +
           link("residents", "/residents", "Residents") +
           link("service-requests", "/service-requests", "Service Requests") + "</nav>";
}
