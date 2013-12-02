<?php
// $Id$

// protected _message_user_center_message_outbox($center) {{{ 
/**
 * _message_user_center_message_outbox
 *  发件箱
 * 
 * @param object $center 
 * @access protected
 * @return void
 */
function _message_user_center_message_outbox($center) {
  if ($fetch = pager_query('SELECT mid, title, timestamp, count, users FROM {message_info} 
  WHERE uid = ? AND status = 1 ORDER BY timestamp DESC', 20, 0, NULL, array($center->user->uid))) {
    $table = array();
    foreach ($fetch as $o)  {
      $table[] = array(
        l($o->title ? $o->title : t('message', '无主题'), 'user/center/message_outbox_view/' . $o->mid),
        $o->users,
        format_date($o->timestamp),
        dd_imp(array(
          l(t('message', '删除'), 'user/center/message_outbox_remove/' . $o->mid, array(
            'attributes' => array(
              'class' => 'confirmajax',
              'method' => 'POST'
            )
          )),
          l(t('message', '转发'), 'user/center/message_form', array('query' => 'op=refer&mid=' . $o->mid)),
        ))
      );
    }
    $header = array(
      t('message', '主题'),
      t('message', '收件人'),
      t('message', '发送时间'),
      t('message', '操作'),
    );
    $center->body = theme('table', $header, $table, array('class' => 'center_table'));
  } else {
    $center->body = system_no_content();
  }
}
// }}}

// protected _message_user_center_message_inbox($center) {{{ 
/**
 * _message_user_center_message_inbox
 *  收件箱
 * 
 * @param object $center 
 * @access protected
 * @return void
 */
function _message_user_center_message_inbox($center) {
  if ($fetch = pager_query('SELECT id, timestamp, mid, status FROM {message} WHERE touid = ? AND tostatus = 0 
  ORDER BY timestamp DESC', 20, 0, NULL, array($center->user->uid))) {
    $ids = array();
    $data = array();
    foreach ($fetch as $o) {
      $data[$o->mid] = $o;
      $ids[] = $o->mid;
    }
    
    if ($f = db_query('SELECT m.mid, m.title, u.name  FROM {message_info} m INNER JOIN {users} u ON m.uid = u.uid 
    WHERE m.mid IN (:id)', array(':id' => $ids))) {
      foreach ($f as $o) {
        $data[$o->mid]->title = $o->title;
        $data[$o->mid]->name = $o->name;
      }
    }

    $header = array();
    $table = array();
    foreach ($data as $o)  {
      $table[] = array(
        l($o->title ? $o->title : t('message', '无主题'), 'user/center/message_inbox_view/' . $o->id),
        $o->name,
        array(
          'data' => $o->status ? t('message', '已读') : t('message', '未读'),
          'class' => 'message_inbox_list_status message_inbox_list_status_' . $o->status,
        ),
        format_date($o->timestamp),
        dd_imp(array(
          l(t('message', '转发'), 'user/center/message_form', array('query' => 'op=refer&id=' . $o->id)),
          l(t('message', '回复'), 'user/center/message_form', array('query' => 'op=reply&id=' . $o->id)),
          l(t('message', '删除'), 'user/center/message_inbox_remove/' . $o->id, array(
            'attributes' => array(
              'class' => 'confirmajax',
              'method' => 'POST'
            )
          )),
        ))
      );
    }
    $header = array(
      t('message', '主题'),
      t('message', '发件人'),
      t('message', '状态'),
      t('message', '发送时间'),
      t('message', '操作'),
    );
    $center->body = theme('table', $header, $table, array('class' => 'center_table'));
  } else {
    $center->body = system_no_content();
  }
}
// }}}

function _message_user_center_message_draft_box($center) {
  $center->tabs = _message_user_center_tabs();
  if ($fetch = pager_query('SELECT mid, title, timestamp FROM {message_info} 
  WHERE uid = ? AND status = 0 ORDER BY timestamp DESC', 20, 0, NULL, array($center->user->uid))) {
    $table = array();
    foreach ($fetch as $o)  {
      $table[] = array(
        $o->title,
        format_date($o->timestamp),
        dd_imp(array(
          l(t('message', '编辑并发送'), 'user/center/message_form', array('query' => 'op=draft&mid=' . $o->mid)),
          l(t('message', '删除'), 'user/center/message_draft_delete/' . $o->mid, array(
           'attributes' => array(
              'class' => 'confirmajax',
              'method' => 'POST'
            )
          )),
        ))
      );
    }
    $header = array(
      t('message', '主题'),
      t('message', '创建时间'),
      t('message', '操作'),
    );
    $center->body = theme('table', $header, $table, array('class' => 'center_table'));
  } else {
    $center->body = system_no_content();
  }
}

// protected _message_user_center_message_form($center) {{{ 
/**
 * _message_user_center_message_form
 *  短信编辑表单
 * 
 * @param object $center 
 * @access protected
 * @return void
 */
function _message_user_center_message_form($center) {
  $center->body = dd_get_form('message_send_form', $center->user);
  $center->tabs = _message_user_center_tabs();
}
// }}}

// protected _message_user_center_message_outbox_remove($center) {{{ 
/**
 * _message_user_center_message_outbox_remove
 *  从发件箱删除一条短信
 * 
 * @param object $center 
 * @access protected
 * @return void
 */
function _message_user_center_message_outbox_remove($center) {
  if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    $mid = arg(3);
    if (message_outbox_remove_validate($mid, $center->user)) {
      if (message_outbox_remove($mid)) {
        echo 'tr';
        exit;
      }
    }
  }

  echo t('message', '删除失败，请重试');
  exit;
}
// }}}

// protected _message_user_center_message_inbox_remove($center) {{{ 
/**
 * _message_user_center_message_inbox_remove
 *  从收件箱移除一条短信
 * 
 * @param object $center 
 * @access protected
 * @return void
 */
function _message_user_center_message_inbox_remove($center) {
  if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    $id = arg(3);
    if (message_inbox_remove_validate($id, $center->user)) {
      if (message_inbox_remove($id)) {
        echo 'tr';
        exit;
      }
    }
  }

  echo t('message', '删除失败，请重试');
  exit;
}
// }}}

// protected _message_user_center_message_restore($center) {{{ 
/**
 * _message_user_center_message_restore
 *  从回收站里恢复一条短信
 * 
 * @param object $center 
 * @access protected
 * @return void
 */
function _message_user_center_message_restore($center) {
  if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    if (db_exec('UPDATE {message} SET tostatus = 0 WHERE id = ? AND touid = ? AND tostatus = 1',
    array(arg(3), $center->user->uid))) {
      message_update_count('get', $center->user->uid);
      echo 'tr';
      exit;
    }
  }

  echo t('message', '恢复失败，请重试');
  exit;
}
// }}}

// protected _message_user_center_message_recycle_delete($center) {{{ 
/**
 * _message_user_center_message_recycle_delete
 *  从回收站内删除一条短信，删除后不可恢复，收件方无法再浏览
 * 
 * @param object $center 
 * @access protected
 * @return void
 */
function _message_user_center_message_recycle_delete($center) {
  if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    if (db_exec('UPDATE {message} SET tostatus = 2 WHERE id = ? AND touid = ? AND tostatus = 1',
    array(arg(3), $center->user->uid))) {
      message_update_count('get', $center->user->uid);
      echo 'tr';
      exit;
    }
  }

  echo t('message', '删除失败，请重试');
  exit;
}
// }}}

// protected _message_user_center_message_draft_delete($center) {{{ 
/**
 * _message_user_center_message_draft_delete
 *  删除草稿，草稿不进入回收站，直接删除，无法恢复
 * 
 * @param object $center 
 * @access protected
 * @return void
 */
function _message_user_center_message_draft_delete($center) {
  if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    if (db_exec('DELETE FROM {message_info} WHERE mid = ? AND uid = ? AND status = 0',
    array(arg(3), $center->user->uid))) {
      message_update_count('set', $center->user->uid);
      dd_log('message', t('message', '删除草稿(id：!id)', array('!id' => arg(3))));
      echo 'tr';
      exit;
    }
  }

  echo t('message', '删除失败，请重试');
  exit;
}

// }}}
// protected _message_user_center_message_recycle_box($center) {{{ 
/**
 * _message_user_center_message_recycle_box
 *  回收站，从收件箱中移除的短信将进入回收站，回收站短信可恢复
 *  若从回收站内删除，删除方将再也无法浏览该条短信
 *  发件箱内的短信移除后不会进入回收站
 * 
 * @param object $center 
 * @access protected
 * @return void
 */
function _message_user_center_message_recycle_box($center) {
  if ($fetch = pager_query('SELECT id, timestamp, mid, status FROM {message} WHERE touid = ? AND tostatus = 1 
  ORDER BY timestamp DESC', 20, 0, NULL, array($center->user->uid))) {
    $ids = array();
    $data = array();
    foreach ($fetch as $o) {
      $data[$o->mid] = $o;
      $ids[] = $o->mid;
    }
    
    if ($f = db_query('SELECT m.mid, m.title, u.name  FROM {message_info} m INNER JOIN {users} u ON m.uid = u.uid 
    WHERE m.mid IN (:id)', array(':id' => $ids))) {
      foreach ($f as $o) {
        $data[$o->mid]->title = $o->title;
        $data[$o->mid]->name = $o->name;
      }
    }

    $header = array();
    $table = array();
    foreach ($data as $o)  {
      $table[] = array(
        $o->title ? $o->title : t('message', '无主题'),
        $o->name,
        array(
          'data' => $o->status ? t('message', '已读') : t('message', '未读'),
          'class' => 'message_inbox_list_status message_inbox_list_status_' . $o->status,
        ),
        format_date($o->timestamp),
        dd_imp(array(
          l(t('message', '彻底删除'), 'user/center/message_recycle_delete/' . $o->id, array(
            'attributes' => array(
              'class' => 'confirmajax',
              'method' => 'POST'
            )
          )),
          l(t('message', '恢复'), 'user/center/message_restore/' . $o->id, array(
            'attributes' => array(
              'class' => 'confirmajax',
              'method' => 'POST'
            )
          )),
        ))
      );
    }
    $header = array(
      t('message', '主题'),
      t('message', '发件人'),
      t('message', '状态'),
      t('message', '发送时间'),
      t('message', '操作'),
    );
    $center->body = theme('table', $header, $table, array('class' => 'center_table'));
  } else {
    $center->body = system_no_content();
  }

}
// }}}

function _message_user_center_message_outbox_view($center) {
  $data = message_load_info(arg(3));
  if (empty($data) || $data->uid != $center->user->uid || $data->status != 1) {
    return dd_get_access();
  }

  $center->tabs[] = l(t('message', '发信浏览'), $_GET['q']);

  $users = '';
  if ($data->count < 10) {
    $users = $data->users;
  } else {
    if ($fetch = db_query('SELECT u.name FROM {message} m INNER JOIN {users} u 
    ON m.touid = u.uid WHERE m.mid = ?', array($data->mid))) {
      foreach ($fetch as $o) {
        $users .= $o->name . ',';
      }
      $users .= rtrim($users, ',');
    }
  }

  $item = array(
    array(
      'data' => dd_get_strong(t('message', '主题')) . $data->title,
      'class' => 'title'
    ),
    array(
      'data' => dd_get_strong(t('message', '收件人')) . $users,
      'class' => 'user',
    ),
    array(
      'data' => dd_get_strong(t('message', '发送时间')) . format_date($data->timestamp),
      'class' => 'time',
    ),
    array(
      'data' => filter_view($data->body),
      'class' => 'body'
    )
  );

  $center->body = theme('item_list', $item, NULL, 'ul', array('id' => 'message_outbox_view_wrapper'));
}

function _message_user_center_message_inbox_view($center) {
  $data = message_load_inbox(arg(3));
  if (empty($data) || $data->touid != $center->user->uid || $data->tostatus != 0) {
    return dd_get_access();
  }

  $center->tabs[] = l(t('message', '收信浏览'), $_GET['q']);

  $item = array(
    array(
      'data' => dd_get_strong(t('message', '主题')) . $data->info->title,
      'class' => 'title'
    ),
    array(
      'data' => dd_get_strong(t('message', '发件人')) . $data->info->name,
      'class' => 'user',
    ),
    array(
      'data' => dd_get_strong(t('message', '发送时间')) . format_date($data->timestamp),
      'class' => 'time',
    ),
    array(
      'data' => filter_view($data->info->body),
      'class' => 'body'
    )
  );

  // 修改短信阅读状态
  if (empty($data->status)) {
    if (db_exec('UPDATE {message} SET status = 1 WHERE id = ?', array($data->id))) {
      message_update_count('new', $center->user->uid);
    }
  }

  $center->body = theme('item_list', $item, NULL, 'ul', array('id' => 'message_inbox_view_wrapper'));

  $_GET['op'] = 'reply';
  $_GET['id'] = $data->id;
  $center->body .= dd_get_form('message_send_form', $center->user);
}
