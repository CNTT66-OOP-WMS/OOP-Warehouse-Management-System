// ========================================================
// CORE JAVASCRIPT: KET NOI REST API BACKEND C++
// ========================================================

const API_BASE = '';

function dinhDangTien(soTien) {
    if (!soTien) return '0 ₫';
    return Number(soTien).toLocaleString('vi-VN') + ' ₫';
}

function layGioHang() {
    const gh = localStorage.getItem('wms_cart');
    return gh ? JSON.parse(gh) : [];
}

function luuGioHang(gh) {
    localStorage.setItem('wms_cart', JSON.stringify(gh));
    capNhatSoLuongGioHangUI();
}

function themVaoGioHang(maSP, tenSP, giaBan, loaiSP) {
    let gh = layGioHang();
    let item = gh.find(x => x.maSP === maSP);
    if (item) {
        item.soLuong += 1;
    } else {
        gh.push({ maSP, tenSP, giaBan, loaiSP, soLuong: 1 });
    }
    luuGioHang(gh);
    alert('Đã thêm sản phẩm "' + tenSP + '" vào giỏ hàng!');
}

function capNhatSoLuongGioHangUI() {
    const badge = document.getElementById('cart-count-badge');
    if (badge) {
        const gh = layGioHang();
        const total = gh.reduce((sum, item) => sum + item.soLuong, 0);
        badge.innerText = total;
        badge.style.display = total > 0 ? 'inline-block' : 'none';
    }
}

// ========================================================
// GIOI THIEU THU THAP CANH BAO OBSERVER PATTERN
// ========================================================
async function taiCanhBaoObserver() {
    const user = Auth.layNguoiDungHienTai();
    if (!user || user.vaiTro === 'KHACH_HANG') return;

    try {
        const [resCanhBao, resSanPham] = await Promise.all([
            fetch('/api/kho/canh-bao'),
            fetch('/api/san-pham')
        ]);
        const [data, dataSanPham] = await Promise.all([resCanhBao.json(), resSanPham.json()]);
        const banner = document.getElementById('observer-alert-banner');
        const countBadge = document.getElementById('alert-count-badge');
        
        if (data.thanhCong && data.danhSach && data.danhSach.length > 0) {
            const sanPhamTheoMa = new Map((dataSanPham.danhSach || []).map(sp => [sp.maSP, sp]));
            const canhBaoConHieuLuc = data.danhSach.filter(cb => {
                const sp = sanPhamTheoMa.get(cb.maSP);
                if (!sp) return true;
                if (cb.loaiCanhBao === 'HET_HANG') return Number(sp.soLuongTon) <= Number(sp.tonToiThieu);
                if (cb.loaiCanhBao === 'SAP_HET_HAN') return !!sp.sapHetHan;
                return true;
            });
            const chuaDoc = canhBaoConHieuLuc.filter(x => !x.daDoc);
            if (countBadge) {
                countBadge.innerText = chuaDoc.length;
                countBadge.style.display = chuaDoc.length > 0 ? 'inline-block' : 'none';
            }

            if (banner && chuaDoc.length > 0) {
                const coCanhBaoHanDung = chuaDoc.some(x => x.loaiCanhBao === 'SAP_HET_HAN');
                banner.className = `alert-box ${coCanhBaoHanDung ? 'alert-danger' : 'alert-warning'}`;
                banner.style.display = 'flex';
                banner.style.flexWrap = 'wrap';
                banner.innerHTML = `
                    <div style="display:grid; gap:6px; flex:1; min-width:250px;">
                        ${chuaDoc.map(cb => {
                            const laCanhBaoHanDung = cb.loaiCanhBao === 'SAP_HET_HAN';
                            const icon = laCanhBaoHanDung ? '⏳ [Cảnh báo hạn dùng - FEFO]' : '⚠️ [Cảnh báo tồn kho]';
                            return `<div><strong>${icon} Observer Pattern:</strong> ${cb.noiDung}</div>`;
                        }).join('')}
                    </div>
                    <button class="btn btn-outline btn-sm" onclick="danhDauDaDocTatCaCanhBao()">Đánh dấu đã xem (${chuaDoc.length})</button>
                `;
            } else if (banner) {
                banner.style.display = 'none';
            }
        } else {
            const banner = document.getElementById('observer-alert-banner');
            const countBadge = document.getElementById('alert-count-badge');
            if (banner) banner.style.display = 'none';
            if (countBadge) countBadge.style.display = 'none';
        }
    } catch (e) {
        console.error('Loi tai canh bao:', e);
    }
}

async function danhDauDaDocTatCaCanhBao() {
    try {
        await fetch('/api/kho/canh-bao/da-doc', { method: 'POST' });
        const banner = document.getElementById('observer-alert-banner');
        if (banner) banner.style.display = 'none';
        const countBadge = document.getElementById('alert-count-badge');
        if (countBadge) countBadge.style.display = 'none';
    } catch (e) {
        console.error(e);
    }
}

document.addEventListener('DOMContentLoaded', () => {
    capNhatSoLuongGioHangUI();
    taiCanhBaoObserver();
    // Cap nhat banner theo ton kho moi, ke ca khi thay doi tu tab/nguoi dung khac.
    window.setInterval(() => {
        if (!document.hidden) taiCanhBaoObserver();
    }, 3000);
});

