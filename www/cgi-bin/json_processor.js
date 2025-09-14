const fs = require('fs');
const os = require('os');
const crypto = require('crypto');
const querystring = require('querystring');

// Function to get posted data
function getPostedData(callback) {
    const method = process.env.REQUEST_METHOD || '';
    const contentType = process.env.CONTENT_TYPE || '';
    
    if (method.toUpperCase() !== 'POST') {
        callback(null, null, '');
        return;
    }
    
    let data = '';
    process.stdin.on('data', chunk => {
        data += chunk;
    });
    
    process.stdin.on('end', () => {
        let jsonText = '';
        if (contentType.includes('application/json')) {
            try {
                const jsonData = JSON.parse(data);
                jsonText = JSON.stringify(jsonData, null, 2);
                callback(null, jsonData, jsonText);
            } catch (e) {
                callback(e, null, '');
            }
        } else if (contentType.includes('application/x-www-form-urlencoded')) {
            try {
                const parsed = querystring.parse(data);
                if (parsed.json) {
                    const jsonData = JSON.parse(parsed.json);
                    jsonText = parsed.json;
                    callback(null, jsonData, jsonText);
                } else {
                    callback(new Error('No JSON data found in form'), null, '');
                }
            } catch (e) {
                callback(e, null, '');
            }
        } else {
            callback(new Error('Unsupported content type'), null, '');
        }
    });
}

function analyzeData(data) {
    const analysis = {
        userStats: {
            total: data.users ? data.users.length : 0,
            byRole: {}
        },
        productStats: {
            total: data.products ? data.products.length : 0,
            totalValue: data.products ? data.products.reduce((sum, product) => sum + (product.price * product.stock), 0) : 0,
            averagePrice: data.products ? data.products.reduce((sum, product) => sum + product.price, 0) / data.products.length : 0
        },
        orderStats: {
            total: data.orders ? data.orders.length : 0,
            totalRevenue: data.orders ? data.orders.reduce((sum, order) => sum + order.total, 0) : 0,
            byStatus: {}
        }
    };

    // Analyze users by role
    if (data.users) {
        data.users.forEach(user => {
            analysis.userStats.byRole[user.role] = (analysis.userStats.byRole[user.role] || 0) + 1;
        });
    }

    // Analyze orders by status
    if (data.orders) {
        data.orders.forEach(order => {
            analysis.orderStats.byStatus[order.status] = (analysis.orderStats.byStatus[order.status] || 0) + 1;
        });
    }

    return analysis;
}

// Process the request
getPostedData((err, postedData, jsonText) => {
    const dataToProcess = postedData || {
        users: [
            { id: 1, name: "Alice Johnson", email: "alice@example.com", role: "admin", lastLogin: "2024-01-15T10:30:00Z" },
            { id: 2, name: "Bob Smith", email: "bob@example.com", role: "user", lastLogin: "2024-01-14T15:45:00Z" },
            { id: 3, name: "Carol Davis", email: "carol@example.com", role: "moderator", lastLogin: "2024-01-13T09:20:00Z" },
            { id: 4, name: "David Wilson", email: "david@example.com", role: "user", lastLogin: "2024-01-12T14:10:00Z" },
            { id: 5, name: "Eve Brown", email: "eve@example.com", role: "admin", lastLogin: "2024-01-11T11:55:00Z" }
        ],
        products: [
            { id: "P001", name: "Laptop", price: 999.99, category: "Electronics", stock: 15 },
            { id: "P002", name: "Mouse", price: 29.99, category: "Electronics", stock: 50 },
            { id: "P003", name: "Keyboard", price: 79.99, category: "Electronics", stock: 25 },
            { id: "P004", name: "Monitor", price: 299.99, category: "Electronics", stock: 8 },
            { id: "P005", name: "Headphones", price: 149.99, category: "Audio", stock: 30 }
        ],
        orders: [
            { id: "O001", userId: 1, products: ["P001", "P002"], total: 1029.98, status: "completed" },
            { id: "O002", userId: 2, products: ["P003"], total: 79.99, status: "pending" },
            { id: "O003", userId: 3, products: ["P004", "P005"], total: 449.98, status: "shipped" }
        ]
    };
    
    const analysis = analyzeData(dataToProcess);

    // Generate HTML output
    console.log("Content-Type: text/html");
    console.log("");
    console.log("<!DOCTYPE html>");
    console.log("<html>");
    console.log("<head>");
    console.log("<title>JSON Processor - JavaScript CGI</title>");
    console.log("<style>");
    console.log("body { font-family: 'Segoe UI', sans-serif; background: #1a1a1a; color: #ffffff; margin: 20px; }");
    console.log(".container { max-width: 1200px; margin: 0 auto; background: rgba(255,255,255,0.05); padding: 30px; border-radius: 15px; }");
    console.log("h1 { color: #ffd700; text-align: center; margin-bottom: 30px; }");
    console.log(".section { margin-bottom: 30px; }");
    console.log(".section h2 { color: #ffd700; margin-bottom: 15px; }");
    console.log(".grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; }");
    console.log(".card { background: rgba(255,255,255,0.1); padding: 20px; border-radius: 10px; border-left: 4px solid #ffd700; }");
    console.log(".stat-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); gap: 15px; margin-top: 15px; }");
    console.log(".stat-item { text-align: center; padding: 10px; background: rgba(255,255,255,0.05); border-radius: 8px; }");
    console.log(".stat-number { font-size: 1.5em; color: #ffd700; font-weight: bold; }");
    console.log(".stat-label { color: #cccccc; font-size: 0.9em; margin-top: 5px; }");
    console.log(".table { width: 100%; border-collapse: collapse; margin-top: 15px; }");
    console.log(".table th, .table td { padding: 10px; text-align: left; border-bottom: 1px solid rgba(255,255,255,0.1); }");
    console.log(".table th { background: rgba(255, 215, 0, 0.2); color: #ffd700; font-weight: bold; }");
    console.log(".table tr:hover { background: rgba(255,255,255,0.05); }");
    console.log(".back-btn { background: #666; color: white; padding: 10px 20px; text-decoration: none; border-radius: 5px; display: inline-block; margin-top: 20px; }");
    console.log(".error { color: #ff6b6b; background: rgba(255, 107, 107, 0.1); padding: 15px; border-radius: 8px; margin-bottom: 20px; }");
    console.log("</style>");
    console.log("</head>");
    console.log("<body>");
    console.log("<div class='container'>");
    console.log("<h1>JSON Data Processor</h1>");
    
    // Show error if any
    if (err) {
        console.log(`<div class="error"><strong>Error:</strong> ${err.message}</div>`);
    }
    
    // Upload form
    console.log("<div class='section'>");
    console.log("<h2>Upload JSON Data</h2>");
    console.log("<div class='card'>");
    console.log("<h3 style='color: #ffd700; margin-bottom: 15px;'>Submit JSON for Analysis</h3>");
    console.log("<form method='POST' enctype='application/x-www-form-urlencoded'>");
    console.log(`<textarea name='json' rows='8' style='width:100%; background: rgba(255,255,255,0.05); color: #fff; border: 1px solid rgba(255,255,255,0.1); border-radius: 8px; padding: 10px;' placeholder='Paste your JSON data here'>${jsonText || ''}</textarea>`);
    console.log("<br/><br/>");
    console.log("<button type='submit' class='back-btn'>Analyze JSON</button>");
    console.log("</form>");
    console.log("</div>");
    console.log("</div>");

    // Data Analysis Section
    console.log("<div class='section'>");
    console.log("<h2>Data Analysis Results</h2>");
    console.log("<div class='grid'>");

    // User Statistics
    console.log("<div class='card'>");
    console.log("<h3 style='color: #ffd700; margin-bottom: 15px;'>User Statistics</h3>");
    console.log(`<div class='stat-item'><div class='stat-number'>${analysis.userStats.total}</div><div class='stat-label'>Total Users</div></div>`);
    console.log("<div style='margin-top: 15px;'>");
    console.log("<strong style='color: #ffd700;'>Users by Role:</strong>");
    for (const [role, count] of Object.entries(analysis.userStats.byRole)) {
        console.log(`<div style='margin: 5px 0; color: #cccccc;'>${role}: ${count}</div>`);
    }
    console.log("</div>");
    console.log("</div>");

    // Product Statistics
    console.log("<div class='card'>");
    console.log("<h3 style='color: #ffd700; margin-bottom: 15px;'>Product Statistics</h3>");
    console.log(`<div class='stat-item'><div class='stat-number'>${analysis.productStats.total}</div><div class='stat-label'>Total Products</div></div>`);
    console.log(`<div class='stat-item'><div class='stat-number'>$${analysis.productStats.totalValue.toFixed(2)}</div><div class='stat-label'>Total Inventory Value</div></div>`);
    console.log(`<div class='stat-item'><div class='stat-number'>$${analysis.productStats.averagePrice.toFixed(2)}</div><div class='stat-label'>Average Price</div></div>`);
    console.log("</div>");

    // Order Statistics
    console.log("<div class='card'>");
    console.log("<h3 style='color: #ffd700; margin-bottom: 15px;'>Order Statistics</h3>");
    console.log(`<div class='stat-item'><div class='stat-number'>${analysis.orderStats.total}</div><div class='stat-label'>Total Orders</div></div>`);
    console.log(`<div class='stat-item'><div class='stat-number'>$${analysis.orderStats.totalRevenue.toFixed(2)}</div><div class='stat-label'>Total Revenue</div></div>`);
    console.log("<div style='margin-top: 15px;'>");
    console.log("<strong style='color: #ffd700;'>Orders by Status:</strong>");
    for (const [status, count] of Object.entries(analysis.orderStats.byStatus)) {
        console.log(`<div style='margin: 5px 0; color: #cccccc;'>${status}: ${count}</div>`);
    }
    console.log("</div>");
    console.log("</div>");

    console.log("</div>");
    console.log("</div>");

    // Data Preview Section
    console.log("<div class='section'>");
    console.log("<h2>Data Preview</h2>");
    
    if (dataToProcess.users && dataToProcess.users.length > 0) {
        console.log("<div class='card'>");
        console.log("<h3 style='color: #ffd700; margin-bottom: 15px;'>Users</h3>");
        console.log("<table class='table'>");
        console.log("<thead><tr><th>ID</th><th>Name</th><th>Email</th><th>Role</th><th>Last Login</th></tr></thead>");
        console.log("<tbody>");
        dataToProcess.users.forEach(user => {
            console.log(`<tr><td>${user.id}</td><td>${user.name}</td><td>${user.email}</td><td>${user.role}</td><td>${user.lastLogin}</td></tr>`);
        });
        console.log("</tbody></table>");
        console.log("</div>");
    }
    
    if (dataToProcess.products && dataToProcess.products.length > 0) {
        console.log("<div class='card'>");
        console.log("<h3 style='color: #ffd700; margin-bottom: 15px;'>Products</h3>");
        console.log("<table class='table'>");
        console.log("<thead><tr><th>ID</th><th>Name</th><th>Price</th><th>Category</th><th>Stock</th></tr></thead>");
        console.log("<tbody>");
        dataToProcess.products.forEach(product => {
            console.log(`<tr><td>${product.id}</td><td>${product.name}</td><td>$${product.price.toFixed(2)}</td><td>${product.category}</td><td>${product.stock}</td></tr>`);
        });
        console.log("</tbody></table>");
        console.log("</div>");
    }
    
    if (dataToProcess.orders && dataToProcess.orders.length > 0) {
        console.log("<div class='card'>");
        console.log("<h3 style='color: #ffd700; margin-bottom: 15px;'>Orders</h3>");
        console.log("<table class='table'>");
        console.log("<thead><tr><th>ID</th><th>User ID</th><th>Products</th><th>Total</th><th>Status</th></tr></thead>");
        console.log("<tbody>");
        dataToProcess.orders.forEach(order => {
            console.log(`<tr><td>${order.id}</td><td>${order.userId}</td><td>${order.products.join(', ')}</td><td>$${order.total.toFixed(2)}</td><td>${order.status}</td></tr>`);
        });
        console.log("</tbody></table>");
        console.log("</div>");
    }
    
    console.log("</div>");
    console.log("<a href='/' class='back-btn'>Back to Home</a>");
    console.log("</div>");
    console.log("</body>");
    console.log("</html>");
});
