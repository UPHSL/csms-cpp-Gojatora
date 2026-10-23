#pragma once

#include <drogon/HttpViewData.h>

#include <string>

// Small helpers shared by the Resident views. Presentation only.

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
